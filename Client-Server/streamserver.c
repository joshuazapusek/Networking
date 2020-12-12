#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3940"
#define QUEUE 10

// signal handler
void sigchld_handler(int s) {
    // save error code
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0) {
        errno = saved_errno;
    }
}
// get IP type
void *get_in_addr(struct sockaddr *sa) {
    // IPv4
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }
    // IPv6
    else {
        return &(((struct sockaddr_in6*) sa)->sin6_addr);
    }
}

int main (void) {
    // fds for sockets
    int sockfd, new_fd;  
    // structs for addr info: hints -> server addr info; res -> result addr struct from getaddrinfo; p -> current client
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage their_addr; 
    socklen_t sin_size;
    // signal handler for handling zombie processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    // bool for if socket is clear for port usage
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int returnValue;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    // Get address info from our client
    if ((returnValue = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
        // Error in getaddrinfo with a return value code of _
        fprintf(stderr, "Address info: %s\n", gai_strerror(returnValue));
        return 1;
    }

    // get fd for client
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            // If can't get a socket fd from current address in our list, throw error and continue
            perror("server: socket");
            continue;
        }

        // set a reusable socket so we dont have to wait for clearance in kernel 
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsocketopt");
            exit(1);
        }

        // Try to bind 
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        // We've got a client
        break;
    }

    // Free this data in memory - we've bound so no need
    freeaddrinfo(res);

    // setup listen to client
    if (listen(sockfd, QUEUE) == -1) {
        perror("listen");
        exit(1);
    }

    // Reap those zombies
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // We're now just waiting on client
    printf("Server: waiting...\n");

    while (1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        // Print IP of client
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        printf("server: got connection from %s\n", s);

        // Child process for sending to client
        if (!fork()) {
            close(sockfd);
            if (send(new_fd, "Hello world!", 12, 0) == -1) {
                perror("send");
                exit (0);
            }
            close (new_fd);
        }
    }
    return 0;
}

