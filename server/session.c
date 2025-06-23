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
extern void handle_casino_game(int client_sock, const char *userid);
extern int  get_user_asset(const char *userid);
extern void handle_blackjack_game(int client_sock, const char *userid, CommandType cmd);
extern void handle_race_game(int client_sock, const char *userid);
extern void handle_chat(int client_sock, const char *userid, CommandType cmd);
extern void handle_burger_game(int client_sock, const char *userid);
extern void handle_package_game(int client_sock, const char *userid);
extern void handle_ranking_request(int client_sock);


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
                break;

            case CMD_LOGOUT_REQ:
                money = get_user_asset(userid);
                update_user_asset(userid, money);
                printf("[SERVER] %s logged out; final asset=%d\n", userid, money);
                break;

            case CMD_BLACKJACK_REQ:
            case CMD_BLACKJACK_HIT:
            case CMD_BLACKJACK_RESULT:
                handle_blackjack_game(client_sock, userid, cmd);
                money = get_user_asset(userid);
                update_user_asset(userid, money);
                break;

            case CMD_RACE_REQ:
                handle_race_game(client_sock, userid);
                break;

            case CMD_BURGER_REQ:
            case CMD_BURGER_RES:
                handle_burger_game(client_sock, userid);
                break;
            
            case CMD_PACKAGE_REQ:
            case CMD_PACKAGE_RES:
                handle_package_game(client_sock, userid);
                break;

            case CMD_CHAT_INBOX_REQ:
            case CMD_CHAT_SEND_REQ:
                handle_chat(client_sock, userid, cmd);
                break;
            case CMD_UPDATE_ASSET: {
                AssetUpdateRequest req;
                ssize_t bytes = recv(client_sock,
                                     ((char*)&req) + sizeof(CommandType),
                                     sizeof(AssetUpdateRequest) - sizeof(CommandType),
                                     0);
                if (bytes <= 0) break;

                req.cmd = cmd;

                update_user_asset(req.user_id, req.money);
                printf("[SERVER] %s asset updated: %d\n", req.user_id, req.money);
                break;
            }
            case CMD_RANKING_REQ:
                handle_ranking_request(client_sock);
                break;

            default:
                break;
        }
    }
}
