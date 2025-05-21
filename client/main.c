#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

// 서버 연결
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

// 정상적인 종료 처리
void graceful_exit(int sock) {
    close(sock);
    printf("👋 프로그램을 종료합니다.\n");
    exit(0);
}

// 로그인 루프
int login_loop(int sock) {
    while (1) {
        int login_result = perform_login(sock);

        if (login_result == 1) return 1;
        if (login_result == -1) {
            printf("❌ 서버 응답 오류. 연결 종료합니다.\n");
            return -1;
        }

        // 실패 시 재선택 루프
        int choice = 0;
        char input[10];
        while (1) {
            printf("\n❌ 로그인 실패\n");
            printf("[1] 로그인 다시 시도\n");
            printf("[2] 나가기\n");
            printf("→ 선택: ");
            fgets(input, sizeof(input), stdin);

            if (strlen(input) == 2 && (input[0] == '1' || input[0] == '2')) {
                choice = input[0] - '0';
                break;
            }
            printf("⚠️ 1 또는 2만 정확히 입력해주세요.\n");
        }

        if (choice == 2) return 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    int sock = connect_to_server(argv[1], atoi(argv[2]));
    if (sock < 0) exit(1);

    int result = login_loop(sock);
    if (result == 1) {
        printf("🚀 로그인 성공! 이제 게임이나 채팅을 선택할 수 있어요.\n");
    }

    graceful_exit(sock);
    return 0;
}