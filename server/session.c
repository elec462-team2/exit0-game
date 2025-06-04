// server/session.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/file.h>
#include "../include/protocol.h"
#include "../include/server_api.h"

extern int handle_login(int client_sock, char *user_id_buf);
extern int  get_user_asset(const char *userid);
extern void handle_casino_game(int client_sock, const char *userid);



void update_user_asset(const char *userid, int new_balance) {
    FILE *fp = fopen("data/asset_db.txt", "r");
    if (!fp) return;

    // flock 적용
    int fd = fileno(fp);
    if (flock(fd, LOCK_EX) != 0) {
        fclose(fp);
        perror("flock");
        return;
    }

    FILE *tmp = fopen("data/asset_tmp.txt", "w");
    if (!tmp) { 
        flock(fd, LOCK_UN);
        fclose(fp); 
        return; 
    }

    char line[256], id[MAX_ID_LEN];
    int money;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%19[^:]:%d", id, &money) == 2) {
            if (strcmp(id, userid) == 0)
                fprintf(tmp, "%s:%d\n", id, new_balance);
            else
                fputs(line, tmp);
        }
    }

    fclose(tmp);

    // flock 해제
    flock(fd, LOCK_UN);
    fclose(fp);

    rename("data/asset_tmp.txt", "data/asset_db.txt");
}


void handle_user_commands(int client_sock, const char *userid) {
    int money = get_user_asset(userid);

    while (1) {
        AssetUpdateRequest req;  // 구조체 전체 받기
        ssize_t n = recv(client_sock, &req, sizeof(req), 0);
        if (n <= 0) break;

        switch (req.cmd) {
            case CMD_HIGHLOW_REQ:
                handle_casino_game(client_sock, userid);
                money = get_user_asset(userid);
                update_user_asset(userid, money);
                printf("[SERVER] %s asset saved after casino: %d\n", userid, money);
                break;
            case CMD_UPDATE_ASSET:
                update_user_asset(req.user_id, req.money);
                printf("[SERVER] %s asset updated: %d\n", req.user_id, req.money);
                break;

            case CMD_LOGOUT_REQ:
                update_user_asset(req.user_id, req.money);
                printf("[SERVER] %s logged out; final asset=%d\n", req.user_id, req.money);
                break;

            default:
                printf("[SERVER] Unknown command received: %u\n", req.cmd);
                break;
        }
    }
}
