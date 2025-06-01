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
    int dealer_cards[MAX_CARDS];
    int dealer_card_count;
    int bet;
    int finished;
} BlackjackSession;


void handle_race_game(int client_sock, const char *userid) {
    RaceRequest req;
    recv(client_sock, &req, sizeof(req), 0);

    int selected = req.selected_horse;
    int bet = req.bet;
    int positions[3] = {0, 0, 0};
    const int finish_line = 50;
    int winner = -1;

    int prob[3][10] = {
        {4,4,4,4,4,3,3,3,3,3},  
        {6,6,6,1,1,1,1,1,1,1},  
        {10,1,1,1,1,1,1,1,1,1}   
    };

    srand(time(NULL) ^ getpid());

    while (1) {
        for (int i = 0; i < 3; ++i) {
            positions[i] += prob[i][rand() % 10];
        }

        int finished = 0;
        for (int i = 0; i < 3; ++i) {
            if (positions[i] >= finish_line) {
                winner = i;
                finished = 1;
            }
        }

        RaceStepResponse step = {
            .cmd = CMD_RACE_STEP,
            .finished = finished
        };
        memcpy(step.horse_positions, positions, sizeof(positions));
        send(client_sock, &step, sizeof(step), 0);

        if (finished) break;
        sleep(2);
    }

    int payout = 0;
    if (selected == winner) {
        payout = (winner == 0) ? bet * 1.5 :
                 (winner == 1) ? bet * 2.5 :
                                 bet * 5;
    } else {
        payout = -bet;
    }

    int cur = get_user_asset(userid);
    int new_money = cur + payout;
    update_user_asset(userid, new_money);

    RaceResultResponse res = {
        .cmd = CMD_RACE_END,
        .winner = winner,
        .user_choice = selected,
        .bet = bet,
        .payout = payout,
        .new_money = new_money
    };
    send(client_sock, &res, sizeof(res), 0);

    printf("[SERVER] Race finished: winner=%d, user pick=%d, payout=%d, final=%d\n",
           winner, selected, payout, new_money);
}

void handle_blackjack_game(int client_sock, const char *userid, CommandType cmd) {
    static BlackjackSession session = {0};

    if (cmd == CMD_BLACKJACK_REQ) {
        session.player_score = rand() % 11 + 1 + rand() % 11 + 1;

        // 딜러 카드 초기화
        session.dealer_card_count = 0;
        int d1 = rand() % 11 + 1;
        int d2 = rand() % 11 + 1;
        session.dealer_cards[session.dealer_card_count++] = d1;
        session.dealer_cards[session.dealer_card_count++] = d2;
        session.dealer_score = d1 + d2;
        session.finished = 0;

        BlackjackRequest req;
        recv(client_sock, &req, sizeof(req), 0);
        session.bet = req.bet;

        BlackjackResponse res = {
            .cmd = CMD_BLACKJACK_RES,
            .player_score = session.player_score,
            .dealer_score = d1,  // 한 장만 공개
            .win = -1,
            .is_final = 0,
            .dealer_card_count = 1
        };
        res.dealer_cards[0] = d1;
        send(client_sock, &res, sizeof(res), 0);
    }

    else if (cmd == CMD_BLACKJACK_HIT) {
        int card = rand() % 11 + 1;
        session.player_score += card;

        int is_final = (session.player_score > 21);

        BlackjackResponse res = {
            .cmd = CMD_BLACKJACK_RES,
            .player_score = session.player_score,
            .dealer_score = session.dealer_cards[0], // 여전히 한 장만
            .win = 0,
            .is_final = is_final,
            .dealer_card_count = 1
        };
        res.dealer_cards[0] = session.dealer_cards[0];
        send(client_sock, &res, sizeof(res), 0);
    }

    else if (cmd == CMD_BLACKJACK_RESULT) {
        // 딜러 카드 계속 뽑기
        while (session.dealer_score < 17 && session.dealer_card_count < MAX_CARDS) {
            int card = rand() % 11 + 1;
            session.dealer_cards[session.dealer_card_count++] = card;
            session.dealer_score += card;
        }

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
            .is_final = 1,
            .dealer_card_count = session.dealer_card_count
        };
        memcpy(res.dealer_cards, session.dealer_cards, sizeof(int) * session.dealer_card_count);
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