#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

// gcc -g -Wall -o whodat_client whodat_client.c

#define BUF_SIZE 1024
#define OPTIONS "hp:"
# define SERV_PORT 10001

int main(int argc, char *argv[]) {
    int i = 1;
    int sock_fd;
    int status;
    size_t bytes_read;
    char *user = NULL;
    char *host = NULL;
    char buf[BUF_SIZE] = {0};
    int port = SERV_PORT;
    struct sockaddr_in *serv_addr;
    struct addrinfo *addr_list;
    struct addrinfo *p;
    struct addrinfo hints;;

    { // getopts
        int opt = -1;
        while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
            switch(opt) {
            case 'h':
                printf("%s\n  "
                       "-p #: change port number (defaults to 10,001)\n  "
                       "  -h: display this help menu\n"
                       , argv[0]);
                exit(EXIT_SUCCESS);
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Unknown command line argument; Exiting...\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    // setup hints
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = 0;
    hints.ai_protocol = 0;           // Any protocol
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    // find host and user
    for (i = optind; i < argc; i++) {
        user = strtok(argv[i],  "@");
        host = strtok(NULL, "\n");

        if ((status = getaddrinfo(host, NULL,  &hints, &addr_list)) != 0) {
            fprintf(stderr, "Cannot find host %s\n", host);
            continue;
        }
        p = addr_list;
        serv_addr = (struct sockaddr_in *) p->ai_addr;
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        serv_addr->sin_port = htons(port);

        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) != 0) {
            fprintf(stderr, "%d\n", __LINE__);
            perror("Could not connect");
            close(sock_fd);
            exit(3);
        }
        write(sock_fd,  user, strlen(user));
        memset(buf, 0, BUF_SIZE);

        // read from server response
        while ((bytes_read = read(sock_fd, buf, BUF_SIZE)) > 0) {
            write(STDOUT_FILENO, buf, bytes_read);
            memset(buf, 0, BUF_SIZE);
        }
        close(sock_fd);
        freeaddrinfo(addr_list);
    }
    printf("\n\nClient done, closing...\n");
    return(EXIT_SUCCESS);
}
