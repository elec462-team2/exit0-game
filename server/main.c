#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../include/protocol.h"

extern void handle_login(int client_sock);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {0}, client_addr;
    socklen_t client_len = sizeof(client_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(serv_sock, 5);
    printf("Server listening...\n");

    while (1) {
        int client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) continue;

        pid_t pid = fork();
        if (pid == 0) {
            close(serv_sock);
            handle_login(client_sock);
            close(client_sock);
            exit(0);
        } else {
            close(client_sock);
        }
    }
}
