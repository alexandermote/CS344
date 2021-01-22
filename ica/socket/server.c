#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// gcc -g -Wall -o server server.c

#define LINE_MAX 4096
#define LISTENQ 1

#ifndef SERV_PORT
# define SERV_PORT 10002
#endif // SERV_PORT

int main(int argc, char *argv[]) {
    int listen_fd;
    int sock_fd;
    int i;
    int port = SERV_PORT;
    char buf[LINE_MAX] = {0};
    socklen_t cli_len;
    struct sockaddr_in cli_addr;
    struct sockaddr_in serv_addr;

    if (argc > 1) port = atoi(argv[1]);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
        perror("Bind failed");
        exit(1);
    }
    listen(listen_fd, LISTENQ);
    printf("Server listening on port %d\n", port);

    cli_len = sizeof(cli_addr);
    sock_fd = accept(listen_fd, (struct sockaddr *) &cli_addr, &cli_len);
    while(1) {
        memset(buf, 0,sizeof(buf));
        if((i = read(sock_fd, buf, sizeof(buf))) <= 0) {
            printf("EOF/error found on client connection socket, exiting\n");
            close(sock_fd);
            break;
        }
        fprintf(stdout, "Message from client: <%s>\n", buf);
        write(sock_fd, buf, i);
    }
    printf("socket closed - exiting\n");

    return(EXIT_SUCCESS);
}
