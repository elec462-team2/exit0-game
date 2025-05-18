#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../include/protocol.h"

#define PORT 7777
#define MAX_CLIENT 10

void handle_client(int client_sock);

int main() {
    int serv_sock, client_sock;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_size;

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) {
        perror("socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(serv_sock, MAX_CLIENT) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_size = sizeof(client_addr);
        client_sock = accept(serv_sock, (struct sockaddr*)&client_addr, &client_size);
        if (client_sock == -1) {
            perror("accept");
            continue;
        }

        printf("Client connected.\n");
        pid_t pid = fork();
        if (pid == 0) { // child
            close(serv_sock);
            handle_client(client_sock);
            close(client_sock);
            exit(0);
        } else {
            close(client_sock); // parent
        }
    }

    close(serv_sock);
    return 0;
}

void handle_client(int client_sock) {
    LoginRequest req;
    LoginResponse res;

    ssize_t recv_len = recv(client_sock, &req, sizeof(req), 0);
    if (recv_len <= 0 || req.cmd != CMD_LOGIN_REQ) {
        printf("Invalid request.\n");
        return;
    }

    printf("Login attempt: ID=%s, PW=%s\n", req.user_id, req.password);

    // 임시 처리: ID=admin, PW=1234 일 때 성공
    if (strcmp(req.user_id, "admin") == 0 && strcmp(req.password, "1234") == 0) {
        res.cmd = CMD_LOGIN_RES;
        res.success = 1;
        strcpy(res.message, "Login successful.");
    } else {
        res.cmd = CMD_LOGIN_RES;
        res.success = 0;
        strcpy(res.message, "Login failed.");
    }

    send(client_sock, &res, sizeof(res), 0);
}
