#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// gcc -g -Wall -o client client.c

#define LINE_MAX 4096

#ifndef SERV_PORT
# define SERV_PORT 10002
#endif // SERV_PORT

int main(int argc, char *argv[]) {
    int sock_fd;
    int port = SERV_PORT;
    struct sockaddr_in serv_addr;
    char send_line[LINE_MAX] = {0};
    char recv_line[LINE_MAX] = {0};
    char ip_addr[50] = {0};

    if (argc > 1) strcpy(ip_addr, argv[1]);
    else {
        printf("Must give ip address\n");
        exit(1);
    }
    if (argc > 2) port = atoi(argv[2]);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr.s_addr);
    if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    fputs("ready >> ", stdout);
    while(fgets(send_line, sizeof(send_line), stdin) != NULL) {
        send_line[strlen(send_line) - 1] = 0;
        memset(recv_line, 0, sizeof(recv_line));
        write(sock_fd, send_line, strlen(send_line));
        if (read(sock_fd, recv_line, sizeof(recv_line)) == 0) {
            perror("Socket is closed");
            break;
        }
        fprintf(stdout, "Server response: <%s>\n", recv_line);
        fputs("ready >> ", stdout);
    }

    return(EXIT_SUCCESS);
}
