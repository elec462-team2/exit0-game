//client/casino.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <locale.h>
#include <signal.h>
#include "../include/protocol.h"

static const char *big_digits[10][5] = {
    { " *** ", "*   *", "*   *", "*   *", " *** " },
    { "  *  ", " **  ", "  *  ", "  *  ", " *** " },
    { " *** ", "    *", " *** ", "*    ", "*****" },
    { "**** ", "    *", " *** ", "    *", "**** " },
    { "*  * ", "*  * ", "*****", "   * ", "   * " },    
    { "*****", "*    ", "**** ", "    *", "**** " },
    { " *** ", "*    ", "**** ", "*   *", " *** " },
    { "*****", "   * ", "  *  ", " *   ", " *   " },
    { " *** ", "*   *", " *** ", "*   *", " *** " },
    { " *** ", "*   *", " ****", "    *", " *** " },
};

static void draw_big_number(int num, int row, int col) {
    char s[16];
    sprintf(s, "%d", num);
    for (int i = 0; i < 5; i++) {
        move(row + i, col);
        for (char *p = s; *p; p++) {
            printw("%s ", big_digits[*p - '0'][i]);
        }
    }
}

void play_race_game(int sock, int *money) {
    char buf[16];
    int choice = -1, bet = 0;

    echo(); clear(); box(stdscr, 0, 0);
    mvprintw(2, 5, "** 경마 게임에 오신 걸 환영합니다! **");
    mvprintw(4, 5, "말을 고르고 베팅 금액을 입력하세요.");
    mvprintw(6, 5, "잔고: [%dG]", *money);
    mvprintw(6, 35, "[승률 & 배당률]");
    mvprintw(8, 35, "🐎 : 66.7%% & x1.5");
    mvprintw(9, 35, "🎠 : 40%% & x2.5");
    mvprintw(10, 35, "🐪 : 20%% & x5");
    mvprintw(12, 5, "당신의 말을 고르세요 ( 0: 🐎 1: 🎠 2: 🐪 ) → ");
    getstr(buf); choice = atoi(buf);    

    if (choice < 0 || choice > 2) {
        mvprintw(14, 5, "그런 말은 선택지에 없습니다. * 아무 키나 누르세요... *");
        getch(); return;
    }

    mvprintw(14, 5, "베팅 금액을 입력하세요: ");
    getstr(buf); bet = atoi(buf);
    
    if (bet <= 0 || bet > *money) {
        mvprintw(16, 5, "잘못된 금액이거나 당신의 잔고를 초과한 금액입니다. * 아무 키나 누르세요... *");
        getch(); return;
    }
    noecho();
    CommandType cmd = CMD_RACE_REQ;
    send(sock, &cmd, sizeof(cmd), 0);
    RaceRequest req = { .cmd = CMD_RACE_REQ, .selected_horse = choice, .bet = bet };
    send(sock, &req, sizeof(req), 0);

    const char *horse_names[3] = {"🐎", "🎠", "🐪"};
    int finish_line = 50;

    while (1) {
        RaceStepResponse step;
        ssize_t n = recv(sock, &step, sizeof(step), 0);
        if (n <= 0 || step.cmd != CMD_RACE_STEP) break;

        clear(); box(stdscr, 0, 0);
        for (int i = 0; i < 3; i++) {
            int x = step.horse_positions[i] < finish_line ? step.horse_positions[i] : finish_line;
            mvprintw(2 + i * 3, 2 + x, "%s", horse_names[i]);
            mvprintw(2 + i * 3 + 1, 2 + finish_line, "| END");
            mvhline(2 + i * 3 + 2, 2, '-', finish_line + 5);
        }

        refresh();
        if (step.finished) break;
    }

    RaceResultResponse res;
    recv(sock, &res, sizeof(res), 0);

    mvprintw(14, 5, "경기 종료!");
    mvprintw(15, 5, "당신이 선택한 말: %s (말 %d)", horse_names[res.user_choice], res.user_choice);
    mvprintw(16, 5, "승리한 말: %s (말 %d)", horse_names[res.winner], res.winner);
    mvprintw(17, 5, "배당률: %dG", res.payout);
    mvprintw(18, 5, "경기 종료 후 잔고: %dG", res.new_money);
    *money = res.new_money;

    mvprintw(20, 5, "* 계속하려면 아무 키나 누르세요... *");
    refresh(); getch();
}

void play_blackjack_game(int sock, int *money) {
    char buf[16];
    int bet = 0;

    echo(); clear(); box(stdscr, 0, 0);
    mvprintw(2, 5, "** 블랙잭 게임장에 온 걸 환영합니다! **");
    mvprintw(4, 5, "딜러와 게임을 해봅시다. 총합 21을 넘지 마세요!");
    mvprintw(6, 5, "잔고 : [%dG]", *money);
    mvprintw(8, 5, "베팅 금액을 입력하세요 : ");
    getstr(buf); bet = atoi(buf);
    noecho();

    if (bet <= 0 || bet > *money) {
        mvprintw(10, 5, "잘못된 금액이거나 당신의 잔고를 초과한 금액입니다. * 아무 키나 누르세요... *");
        getch(); return;
    }

    // 서버로 게임 시작 요청s
    CommandType cmd = CMD_BLACKJACK_REQ;
    send(sock, &cmd, sizeof(cmd), 0);
    BlackjackRequest req = { .cmd = CMD_BLACKJACK_REQ, .bet = bet };
    send(sock, &req, sizeof(req), 0);

    BlackjackResponse res;
    recv(sock, &res, sizeof(res), 0);

    int player_score = res.player_score;

    while (1) {
        clear(); box(stdscr, 0, 0);
        mvprintw(1, 5, "[당신]");
        draw_big_number(player_score, 2, 5);
        mvprintw(8, 5, "총합: %d", player_score);

        mvprintw(10, 5, "[h] hit   [s] Stand");
        refresh();
        char choice = getch();

        if (choice == 'h') {
            cmd = CMD_BLACKJACK_HIT;
            send(sock, &cmd, sizeof(cmd), 0);
            recv(sock, &res, sizeof(res), 0);
            player_score = res.player_score;

            if (res.is_final) break;

        } else if (choice == 's') break;
    }
    cmd = CMD_BLACKJACK_RESULT;
    send(sock, &cmd, sizeof(cmd), 0);
    recv(sock, &res, sizeof(res), 0);

    // 최종 결과 화면
    clear(); box(stdscr, 0, 0);
    mvprintw(1, 5, "[결과]");
    draw_big_number(res.player_score, 2, 5);
    mvprintw(8, 5, "당신의 총합: %d", res.player_score);

    mvprintw(1, 35, "[딜러]");
    draw_big_number(res.dealer_score, 2, 35);
    mvprintw(8, 35, "딜러의 총합: %d", res.dealer_score);

    // 딜러가 뽑은 카드 하나씩 보여주기
    int drow = 10;
    mvprintw(drow++, 35, "딜러가 뽑은 카드:");
    for (int i = 0; i < res.dealer_card_count; i++) {
        mvprintw(drow++, 37, "- 카드 %d: %d", i + 1, res.dealer_cards[i]);
    }

    // 메시지 분기
    const char *reason_msg = "";
    if (res.player_score > 21)
        reason_msg = "당신의 수가 21을 넘겼습니다! *딜러*의 승리입니다.";
    else if (res.dealer_score > 21)
        reason_msg = "딜러의 수가 21을 넘겼습니다! *당신*의 승리입니다.";
    else if (res.player_score == res.dealer_score)
        reason_msg = "Push! 무승부입니다.";
    else if (res.player_score > res.dealer_score)
        reason_msg = "당신의 수가 더 높습니다! *당신*의 승리입니다.";
    else
        reason_msg = "딜러의 수가 더 높습니다! *딜러*의 승리입니다.";

    // 결과/자산 반영
    *money = res.new_money;

    mvprintw(drow + 1, 5, "%s", reason_msg);
    mvprintw(drow + 2, 5, "(결과: %s)", res.win == 1 ? "당신이 이겼습니다!" : (res.win == 2 ? "무승부입니다." : "당신이 졌습니다."));
    mvprintw(drow + 4, 5, "최종 자산: %dG", *money);
    mvprintw(drow + 6, 5, "* 계속하려면 아무 키나 누르세요... *");
    refresh(); getch();
}

void play_highlow_game(int sock, int *money) {
    echo();
    char buf[32];
    int bet = 0;
    char guess = 0;

    // 1) TUI로 배팅 입력
    clear(); box(stdscr,0,0);
    mvprintw(2, 5, "** 하이앤로우 게임장에 온 것을 환영합니다! **");
    mvprintw(4, 5, "당신의 숫자가 딜러의 숫자보다 높을 지, 낮을 지 예측하세요.");
    mvprintw(6,5,"잔고 : [%dG]", *money);
    mvprintw(8,5,"베팅 금액을 입력하세요 : ");
    getstr(buf);
    bet = atoi(buf);
    if (bet <= 0 || bet > *money) {
        mvprintw(10,5,"잘못된 금액이거나 당신의 잔고를 초과한 금액입니다. * 아무 키나 누르세요... *");
        getch();
        return;
    }

    // 2) TUI로 H/L 입력
    mvprintw(10,5,"당신은 예측할 수 있을까요? [h] 높다 or [l] 낮다: ");
    getstr(buf);
    noecho();
    refresh();
    guess = buf[0];
    if (guess != 'h' && guess != 'l') {
        mvprintw(12,5,"잘못된 선택입니다. * 아무 키나 누르세요... *");
        getch();
        return;
    }

    int guess_num = (guess == 'h') ? 1 : 0; 

    // 3) 서버에 “페이로드”로 전송
    CommandType hcmd = CMD_HIGHLOW_REQ;
    send(sock, &hcmd, sizeof(hcmd), 0);

    HighLowRequest req = {
        .cmd = CMD_HIGHLOW_REQ,
        .bet = bet,
        .guess_num = guess_num
    };
    send(sock, &req, sizeof(req), 0);

    // 4) 응답 수신
    HighLowResponse res;
    int n = recv(sock, &res, sizeof(res), 0);
    if (n <= 0 || res.cmd != CMD_HIGHLOW_RES) {
        return;
    }
    clear(); box(stdscr, 0, 0);
    mvprintw(2, 5, "당신의 숫자 : %d", res.my_num);      draw_big_number(res.my_num, 4, 5);
    mvprintw(2, 25, "딜러의 숫자 : %d", res.cpu_num); draw_big_number(res.cpu_num, 4,25);
    mvprintw(10, 5, "당신의 예측 : %c", res.guess_num == 1 ? 'H' : 'L');
    mvprintw(12, 5, "결과 : %s (%c%dG)    최종 자산 : %dG", res.win?"WIN":"LOSE", res.win?'+':'-', res.bet, res.new_money);
    mvprintw(16, 5, "* 아무 키나 누르세요... *");

    *money = res.new_money;

    // 5) 아무 키나 누르면 종료
    refresh(); getch();
}

void start_casino_game(int sock, int *user_money) {
    int choice = 0;
    while (1) {
        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "🎰 카지노에 온 것을 환영합니다 🎰");

        if (*user_money < 1) {
            mvprintw(3, 2, "이런, 도박할 돈이 없네요. 노동장에서 돈을 벌어보세요.");
            mvprintw(5, 2, "* 로비로 돌아가려면 아무 키나 누르세요... *");
            refresh();
            getch();
            return;
        }

        mvprintw(3, 2, "잔고: $%d", *user_money);
        mvprintw(5, 2, "[1] 하이 앤 로우 게임");
        mvprintw(6, 2, "[2] 블랙잭 게임");
        mvprintw(7, 2, "[3] 경마 게임");
        mvprintw(8, 2, "[4] 게임 규칙 설명");
        mvprintw(9, 2, "[Q] 로비로 나가기");
        mvprintw(11, 2, "→ 선택지를 입력하세요: ");
        char buf[8];
        echo();
        getstr(buf);
        noecho();
        refresh();

        // 🔍 Q/q 입력 시 종료 처리
        if (strlen(buf) == 1 && (buf[0] == 'q' || buf[0] == 'Q')) {
            return;
        }

        choice = atoi(buf);
        switch (choice) {
            case 1:
                play_highlow_game(sock, user_money);
                break;
            case 2:
                play_blackjack_game(sock, user_money);
                break;
            case 3:
                play_race_game(sock, user_money);
                break;
            case 4:
                clear();
                box(stdscr, 0, 0);
                mvprintw(1, 2, "[ 🎮 게임 규칙 설명서 ]");

                // High & Low
                mvprintw(3, 2, "① 하이⤴️  & 로우⤵️ (난이도: 하)");
                mvprintw(4, 4, "- 당신의 숫자가 딜러의 숫자보다 클 지, 작을 지 맞히는 게임입니다.");
                mvprintw(5, 4, "- 숫자는 1부터 100 사이에서 무작위로 생성됩니다.");

                // Blackjack
                mvprintw(8, 2, "② 블랙잭 🃏 (난이도: 중)");
                mvprintw(9, 4, "- 당신과 딜러가 카드를 뽑으며 21에 가까운 수를 만드는 게임입니다.");
                mvprintw(10, 4, "- 당신의 점수가 21을 넘으면 즉시 패배합니다.");
                mvprintw(11, 4, "- 딜러보다 높고 21 이하일 경우 승리하여 보상을 얻습니다.");

                // Race (경마)
                mvprintw(14, 2, "③ 경마 🐎 (난이도: 상)");
                mvprintw(15, 4, "- 승률이 다른 3마리의 말 중 하나를 선택합니다.");
                mvprintw(16, 4, "- 선택한 말이 1등으로 도착하면 배당률에 따라 보상을 받습니다.");
                mvprintw(17, 4, "- 승률이 낮을 말일수록 이겼을 때의 보상이 큽니다.");

                mvprintw(20, 2, "* 아무 키나 눌러서 메인 메뉴로 돌아가세요... *");
                refresh();
                getch();
                break;
            default:
                mvprintw(13, 2, "잘못된 입력입니다. 다시 입력하세요. ");
                refresh();
                getch();
                break;
        }
    }
}