#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../include/protocol.h"
#include "../include/server_api.h"

// 아이디+비번 일치 확인 (로그인용)
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

// 아이디만 중복 여부 확인 (회원가입용)
int check_user_id_exists(const char *userid) {
    FILE *fp = fopen("data/user_db.txt", "r");
    if (!fp) return 0;

    char line[100], file_id[50], file_pw[50];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%49[^:]:%49s", file_id, file_pw) == 2) {
            if (strcmp(userid, file_id) == 0) {
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

// 클라이언트 요청 처리 (로그인/회원가입)
void handle_login(int client_sock) {
    while (1) {
        RegisterRequest req;
        ssize_t recv_len = recv(client_sock, &req, sizeof(req), 0);
        if (recv_len <= 0) break;

        if (req.cmd == CMD_LOGIN_REQ) {
            LoginResponse res;
            res.cmd = CMD_LOGIN_RES;

            if (check_user_credentials(req.user_id, req.password)) {
                res.success = 1;
                res.money = get_user_asset(req.user_id);
                snprintf(res.message, MAX_MSG_LEN, "Login success, welcome %s!", req.user_id);
            } else {
                res.success = 0;
                res.money = 0;
                snprintf(res.message, MAX_MSG_LEN, "Login failed.");
            }

            send(client_sock, &res, sizeof(res), 0);
        } 
        else if (req.cmd == CMD_REGISTER_REQ) {
            RegisterResponse res;
            res.cmd = CMD_REGISTER_RES;

            if (strlen(req.password) == 0) {  // 첫 요청: ID 중복 확인
                if (check_user_id_exists(req.user_id)) {
                    res.success = 0;
                    snprintf(res.message, MAX_MSG_LEN, "ID already exists.");
                } else {
                    res.success = 1;
                    snprintf(res.message, MAX_MSG_LEN, "ID is available.");
                }
            } else {  // 두 번째 요청: 최종 등록 (ID + 비번 + 초기 자산)
                FILE *fp = fopen("data/user_db.txt", "a");
                fprintf(fp, "%s:%s\n", req.user_id, req.password);
                fclose(fp);

                FILE *afp = fopen("data/asset_db.txt", "a");
                fprintf(afp, "%s:%d\n", req.user_id, 0);
                fclose(afp);

                res.success = 1;
                snprintf(res.message, MAX_MSG_LEN, "Registration complete.");
            }

            send(client_sock, &res, sizeof(res), 0);
        }

    }
}

// 자산 정보 가져오기
int get_user_asset(const char *userid) {
    FILE *fp = fopen("data/asset_db.txt", "r");
    if (!fp) return 0;

    char line[100], file_id[50];
    int money;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%49[^:]:%d", file_id, &money) == 2) {
            if (strcmp(userid, file_id) == 0) {
                fclose(fp);
                return money;
            }
        }
    }
    fclose(fp);
    return 0;
}
