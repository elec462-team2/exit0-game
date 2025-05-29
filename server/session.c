// server/session.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../include/protocol.h"
#include "../include/server_api.h"

extern int handle_login(int client_sock, char *user_id_buf);
extern int  get_user_asset(const char *userid);

// 필요 없으면 주석처리하거나 빈 함수
void handle_casino_game(int client_sock, const char *userid) {
    // Do nothing or implement if necessary
    printf("[SERVER] Casino game not implemented on server side.\n");
}

void update_user_asset(const char *userid, int new_balance) {
    FILE *fp = fopen("data/asset_db.txt", "r");
    if (!fp) return;
    FILE *tmp = fopen("data/asset_tmp.txt", "w");
    if (!tmp) { fclose(fp); return; }

    char line[256], id[MAX_ID_LEN];
    int money;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%19[^:]:%d", id, &money) == 2) {
            if (strcmp(id, userid) == 0)
                fprintf(tmp, "%s:%d\n", id, new_balance);
            else fputs(line, tmp);
        }
    }

    fclose(fp); fclose(tmp);
    rename("data/asset_tmp.txt", "data/asset_db.txt");
}

void handle_user_commands(int client_sock, const char *userid) {
    int money = get_user_asset(userid);

    while (1) {
        CommandType cmd;
        ssize_t n = recv(client_sock, &cmd, sizeof(cmd), 0);
        if (n <= 0) break;

        switch (cmd) {
            case CMD_HIGHLOW_REQ:
                handle_casino_game(client_sock, userid);
                money = get_user_asset(userid);
                update_user_asset(userid, money);
                printf("[SERVER] %s asset saved after casino: %d\n", userid, money);
                break;

            case CMD_LOGOUT_REQ:
                money = get_user_asset(userid);
                update_user_asset(userid, money);
                printf("[SERVER] %s logged out; final asset=%d\n", userid, money);
                break;

            default:
                break;
        }
    }
}