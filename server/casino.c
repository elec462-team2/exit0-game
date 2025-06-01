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

typedef struct {
    int player_score;
    int dealer_score;
    int bet;
    int finished;
} BlackjackSession;

void handle_blackjack_game(int client_sock, const char *userid, CommandType cmd) {
    static BlackjackSession session = {0};

    if (cmd == CMD_BLACKJACK_REQ) {
        session.player_score = rand() % 11 + 1 + rand() % 11 + 1;
        session.dealer_score = rand() % 11 + 1 + rand() % 11 + 1;
        session.finished = 0;

        BlackjackRequest req;
        recv(client_sock, &req, sizeof(req), 0);
        session.bet = req.bet;

        BlackjackResponse res = {
            .cmd = CMD_BLACKJACK_RES,
            .player_score = session.player_score,
            .dealer_score = session.dealer_score / 2,  // 한 장만 공개처럼
            .win = -1, .is_final = 0
        };
        send(client_sock, &res, sizeof(res), 0);
    }

    else if (cmd == CMD_BLACKJACK_HIT) {
        int card = rand() % 11 + 1;
        session.player_score += card;

        int is_final = 0;
        if (session.player_score > 21) is_final = 1;

        BlackjackResponse res = {
            .cmd = CMD_BLACKJACK_RES,
            .player_score = session.player_score,
            .dealer_score = session.dealer_score,
            .win = 0, .is_final = is_final
        };
        send(client_sock, &res, sizeof(res), 0);
    }

    else if (cmd == CMD_BLACKJACK_RESULT) {
        while (session.dealer_score < 17)
            session.dealer_score += rand() % 11 + 1;

        int win;
        if (session.player_score > 21) win = 0;
        else if (session.dealer_score > 21 || session.player_score > session.dealer_score) win = 1;
        else if (session.player_score == session.dealer_score) win = 2;
        else win = 0;

        int cur = get_user_asset(userid);
        int new_money = (win == 1) ? cur + session.bet :
                        (win == 0) ? cur - session.bet : cur;
        update_user_asset(userid, new_money);

        BlackjackResponse res = {
            .cmd = CMD_BLACKJACK_RES,
            .player_score = session.player_score,
            .dealer_score = session.dealer_score,
            .win = win,
            .bet = session.bet,
            .new_money = new_money,
            .is_final = 1
        };
        send(client_sock, &res, sizeof(res), 0);
    }
}

// 하이 앤 로우
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