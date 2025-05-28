#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../include/protocol.h"
#include "../include/server_api.h"

int check_user_credentials(const char *userid, const char *password) {
    FILE *fp = fopen("data/user_db.txt", "r");
    if (!fp) return 0;

    char line[100], file_id[50], file_pw[50];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%49[^:]:%49s", file_id, file_pw) == 2) {
            if (strcmp(userid, file_id) == 0 && strcmp(password, file_pw) == 0) {
                fclose(fp);
                return 1;
            }
        }
    }

    fclose(fp);
    return 0;
}

//자산 찾기
int get_user_asset(const char *userid) {
    FILE *fp = fopen("data/asset_db.txt", "r");
    if (!fp) return -1;

    char line[100], file_id[50];
    int money;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%[^:]:%d", file_id, &money) == 2) {
            if (strcmp(userid, file_id) == 0) {
                fclose(fp);
                return money;
            }
        }
    }

    fclose(fp);
    return -1; // 못 찾은 경우
}

int handle_login(int client_sock, char *user_id_buf) {
    while (1) {
        LoginRequest req;
        ssize_t recv_len = recv(client_sock, &req, sizeof(req), 0);
        if (recv_len <= 0) {
            printf("Client disconnected.\n");
            return 0;
        }

        if (req.cmd == CMD_LOGIN_REQ) {
            LoginResponse res;
            res.cmd = CMD_LOGIN_RES;

            if (check_user_credentials(req.user_id, req.password)) {
                res.success = 1;
                res.money = get_user_asset(req.user_id);
                snprintf(res.message, MAX_MSG_LEN, "Login success, welcome %s!", req.user_id);
                strcpy(user_id_buf, req.user_id);
                send(client_sock, &res, sizeof(res), 0);
                return 1; // 로그인 성공
            } else {
                res.success = 0;
                res.money = 0;
                snprintf(res.message, MAX_MSG_LEN, "Login failed.");
                send(client_sock, &res, sizeof(res), 0);
            }
        }
    }
}
