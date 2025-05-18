#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

int connect_to_server(const char *ip, int port) {
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    int sock = connect_to_server(argv[1], atoi(argv[2]));
    if (sock < 0) exit(1);

    if (!perform_login(sock)) {
        close(sock);
        return 1;
    }

    // 로그인 성공 후 메뉴 (미구현)
    printf("🚀 로그인 성공! 이제 게임이나 채팅을 선택할 수 있어요.\n");

    close(sock);
    return 0;
}
