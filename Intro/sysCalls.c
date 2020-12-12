#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// A demo program to preint IP addresses 

// using getaddrinfo function for IP, port #, and addrinfo struct (returns pointer to results array)
//int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);

int main(int argc, char *argv[]) {
    int status;
    // address info structs 
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *p;
    // hold the address
    char ipstr[INET6_ADDRSTRLEN];
    if (argc != 2) {
        fprintf(stderr, "usage: showip hostname\n");
        return 1;
    }
    // set the struct to zeros
    memset(&hints, 0, sizeof(hints));
    // set address info family to unsoec for ipv4 or ipv6
    hints.ai_family = AF_UNSPEC;
    // using tcp
    hints.ai_socktype = SOCK_STREAM;
    // set this computer ip
    hints.ai_flags = AI_PASSIVE;
    // make sure we have executable and ip addr params
    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfp: %s\n", gai_strerror(status));
    }

    printf("IP addresses for %s:\n\n", argv[1]);
    // loop over the different addresses given by getaddrinfo()
    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;
        // Getting pointer to the address
        // IPv4
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        // IPv6
        else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }
        // Print IP by network to presentation
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        printf("%s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(res);
    
    return 0;
}