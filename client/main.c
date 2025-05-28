#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

char global_user_id[MAX_ID_LEN];  // 로그인한 사용자 ID 저장

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
int login_loop(int sock, char *user_id, int *user_money) {
    while (1) {
        int login_result = perform_login(sock, user_id, user_money);

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

    int user_money = 0;
    char user_id[MAX_ID_LEN] = {0};

    int result = login_loop(sock, user_id, &user_money);
    if (result == 1) {
        strcpy(global_user_id, user_id);  // 전역 변수에 저장

        int choice;
        printf("\n🎲 카지노에 입장하시겠습니까?\n");
        printf("[1] 입장하기\n");
        printf("[2] 나가기\n");
        printf("→ 선택: ");
        scanf("%d", &choice);
        getchar();

        if (choice == 1) {
            start_casino_game(sock, &user_money);

            AssetUpdateRequest update_req;
            update_req.cmd = CMD_UPDATE_ASSET;
            update_req.money = user_money;
            strcpy(update_req.user_id, global_user_id);
            send(sock, &update_req, sizeof(update_req), 0);
        }
    }

    // 로그아웃 시 자산 저장 요청
    AssetUpdateRequest logout_req;
    logout_req.cmd = CMD_UPDATE_ASSET;
    logout_req.money = user_money;
    strcpy(logout_req.user_id, global_user_id);
    send(sock, &logout_req, sizeof(logout_req), 0);

    graceful_exit(sock);
    return 0;
}