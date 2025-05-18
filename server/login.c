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

void handle_login(int client_sock) {
    LoginRequest req;
    LoginResponse res;

    ssize_t recv_len = recv(client_sock, &req, sizeof(req), 0);
    if (recv_len <= 0 || req.cmd != CMD_LOGIN_REQ) {
        printf("Invalid login request.\n");
        return;
    }

    printf("Login attempt: ID=%s\n", req.user_id);

    res.cmd = CMD_LOGIN_RES;
    if (check_user_credentials(req.user_id, req.password)) {
        res.success = 1;
        snprintf(res.message, MAX_MSG_LEN, "Login success, welcome %s!", req.user_id);
    } else {
        res.success = 0;
        snprintf(res.message, MAX_MSG_LEN, "Login failed.");
    }

    send(client_sock, &res, sizeof(res), 0);
}
