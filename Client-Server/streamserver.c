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
void *getinaddr(struct sockaddr *sa) {
    // IPv4
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }
    // IPv6
    else {
        return &(((struct sockaddr_in6*) sa)->sin6_addr);
    }
}

