#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../include/protocol.h"

#define SERVER_IP "127.0.0.1"
#define PORT 7777

int main() {
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect");
        exit(1);
    }

    LoginRequest req;
    LoginResponse res;

    req.cmd = CMD_LOGIN_REQ;
    printf("Enter ID: ");
    fgets(req.user_id, MAX_ID_LEN, stdin);
    req.user_id[strcspn(req.user_id, "\n")] = '\0';

    printf("Enter Password: ");
    fgets(req.password, MAX_PW_LEN, stdin);
    req.password[strcspn(req.password, "\n")] = '\0';

    send(sock, &req, sizeof(req), 0);

    ssize_t recv_len = recv(sock, &res, sizeof(res), 0);
    if (recv_len <= 0 || res.cmd != CMD_LOGIN_RES) {
        printf("Login failed: invalid response from server\n");
    } else if (res.success) {
        printf("✅ %s\n", res.message);
    } else {
        printf("❌ %s\n", res.message);
    }

    close(sock);
    return 0;
}
