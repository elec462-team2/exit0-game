// server/casino.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>            // getpid()
#include <sys/socket.h>
#include "../include/protocol.h"
#include "../include/server_api.h"

// 자산 조회·업데이트
extern int  get_user_asset(const char *userid);
extern void update_user_asset(const char *userid, int new_money);

void handle_casino_game(int client_sock, const char *userid) {
    HighLowRequest req;
    ssize_t len = recv(client_sock, &req, sizeof(req), 0);
    if (len <= 0 || req.cmd != CMD_HIGHLOW_REQ) {
        printf("[SERVER] Invalid request or disconnected.\n");
        return;
    }


    srand(time(NULL) ^ getpid());
    int my  = rand() % 100;
    int cpu = rand() % 100;
    int win = (req.guess_num==1 && my>cpu) || (req.guess_num==0 && my<cpu);
    int cur = get_user_asset(userid);
    int upd = win ? cur + req.bet : cur - req.bet;
    update_user_asset(userid, upd);

    HighLowResponse res = {
        .cmd       = CMD_HIGHLOW_RES,
        .my_num    = my,
        .cpu_num   = cpu,
        .win       = win,
        .new_money = upd,
        .guess_num = req.guess_num,
        .bet       = req.bet
        };

    send(client_sock, &res, sizeof(res), 0);
}