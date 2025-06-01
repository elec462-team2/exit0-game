#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../include/protocol.h"
#include "../include/server_api.h"

extern int handle_login(int client_sock, char *user_id_buf);  // 로그인 성공 시 user_id 저장
extern void handle_user_commands(int client_sock, const char *userid);  // 이후 명령 핸들링

// 서버 소캣 생성
int create_server_socket(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(sock, 5) < 0) {
        perror("listen");
        exit(1);
    }

    return sock;
}

// 클라이언트 처리
void accept_and_fork(int serv_sock) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        int client_sock = accept(serv_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) continue;

        pid_t pid = fork();
        if (pid == 0) {
            close(serv_sock);

            char user_id_buf[MAX_ID_LEN] = {0};
            int login_success = handle_login(client_sock, user_id_buf);

            if (login_success == 1) {
                printf("[SERVER] %s logged in successfully\n", user_id_buf);
                handle_user_commands(client_sock, user_id_buf);
            } else {
                printf("[SERVER] Login failed or client disconnected\n");
            }

            close(client_sock);
            exit(0);
        } else {
            close(client_sock);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int serv_sock = create_server_socket(atoi(argv[1]));
    printf("Server listening on port %s...\n", argv[1]);

    accept_and_fork(serv_sock);

    close(serv_sock);
    return 0;
}