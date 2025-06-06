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
    mvprintw(2, 5, "**Welcome to Horse Racing!**");
    mvprintw(4, 5, "Choose your champion and place your bet.");
    mvprintw(6, 5, "Your bank: [%dG]", *money);
    mvprintw(6, 35, "[Win Rate & Payout Rate]");
    mvprintw(8, 35, "(^_^) : 66.7%% & x1.5");
    mvprintw(9, 35, "(>.<) : 40%% & x2.5");
    mvprintw(10, 35, "(-_-) : 20%% & x5");
    mvprintw(12, 5, "Choose your horse (0: (^_^) 1: (>.<), 2: (-_-) : ");
    getstr(buf); choice = atoi(buf);    

    if (choice < 0 || choice > 2) {
        mvprintw(14, 5, "Invalid horse selection. Press any key.");
        getch(); return;
    }

    mvprintw(14, 5, "Enter bet: ");
    getstr(buf); bet = atoi(buf);
    
    if (bet <= 0 || bet > *money) {
        mvprintw(16, 5, "Invalid bet. Press any key.");
        getch(); return;
    }
    noecho();
    CommandType cmd = CMD_RACE_REQ;
    send(sock, &cmd, sizeof(cmd), 0);
    RaceRequest req = { .cmd = CMD_RACE_REQ, .selected_horse = choice, .bet = bet };
    send(sock, &req, sizeof(req), 0);

    const char *horse_names[3] = {"(^_^)", "(>.<)", "(-_-)"};
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

    mvprintw(14, 5, "Race finished!");
    mvprintw(15, 5, "Your choice: %s (Horse %d)", horse_names[res.user_choice], res.user_choice);
    mvprintw(16, 5, "Winner: %s (Horse %d)", horse_names[res.winner], res.winner);
    mvprintw(17, 5, "Payout: %dG", res.payout);
    mvprintw(18, 5, "Bank after race: %dG", res.new_money);
    *money = res.new_money;

    mvprintw(20, 5, "Press any key to continue...");
    refresh(); getch();
}

void play_blackjack_game(int sock, int *money) {
    char buf[16];
    int bet = 0;

    echo(); clear(); box(stdscr, 0, 0);
    mvprintw(2, 5, "**Welcome to Blackjack Table!**");
    mvprintw(4, 5, "Try to beat the dealer. Don’t go over 21!");
    mvprintw(6, 5, "Your bank : [%dG]", *money);
    mvprintw(8, 5, "Enter bet : ");
    getstr(buf); bet = atoi(buf);
    noecho();

    if (bet <= 0 || bet > *money) {
        mvprintw(10, 5, "Invalid bet. Press any key.");
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
        mvprintw(1, 5, "[PLAYER]");
        draw_big_number(player_score, 2, 5);
        mvprintw(8, 5, "Total: %d", player_score);

        mvprintw(10, 5, "[h] Hit   [s] Stand");
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
    mvprintw(1, 5, "[RESULT]");
    draw_big_number(res.player_score, 2, 5);
    mvprintw(8, 5, "Your Total: %d", res.player_score);

    mvprintw(1, 35, "[DEALER]");
    draw_big_number(res.dealer_score, 2, 35);
    mvprintw(8, 35, "Dealer Total: %d", res.dealer_score);

    // 딜러가 뽑은 카드 하나씩 보여주기
    int drow = 10;
    mvprintw(drow++, 35, "Dealer's draw:");
    for (int i = 0; i < res.dealer_card_count; i++) {
        mvprintw(drow++, 37, "- Card %d: %d", i + 1, res.dealer_cards[i]);
    }

    // 메시지 분기
    const char *reason_msg = "";
    if (res.player_score > 21)
        reason_msg = "You busted! Dealer wins.";
    else if (res.dealer_score > 21)
        reason_msg = "Dealer busted! You win.";
    else if (res.player_score == res.dealer_score)
        reason_msg = "Push. It's a tie.";
    else if (res.player_score > res.dealer_score)
        reason_msg = "You win! Your score is higher.";
    else
        reason_msg = "Dealer wins. Higher score.";

    // 결과/자산 반영
    *money = res.new_money;

    mvprintw(drow + 1, 5, "%s", reason_msg);
    mvprintw(drow + 2, 5, "(Result: %s)", res.win == 1 ? "You WIN!" : (res.win == 2 ? "Push." : "You LOSE!"));
    mvprintw(drow + 4, 5, "Final bank: %dG", *money);
    mvprintw(drow + 6, 5, "Press any key to return.");
    refresh(); getch();
}

void play_highlow_game(int sock, int *money) {
    echo();
    char buf[32];
    int bet = 0;
    char guess = 0;

    // 1) TUI로 배팅 입력
    clear(); box(stdscr,0,0);
    mvprintw(2, 5, "**Welcome to High & Low Game!**안녕하세요!🐕");
    mvprintw(4, 5, "Guess whether the next number is HIGH or LOW.");
    mvprintw(6,5,"Your bank : [%dG]", *money);
    mvprintw(8,5,"Enter bet : ");
    getstr(buf);
    bet = atoi(buf);
    if (bet <= 0 || bet > *money) {
        mvprintw(10,5,"Invalid bet. Press any key.");
        getch();
        return;
    }

    // 2) TUI로 H/L 입력
    mvprintw(10,5,"Guess [h] HIGH or [l] LOW: ");
    getstr(buf);
    noecho();
    refresh();
    guess = buf[0];
    if (guess != 'h' && guess != 'l') {
        mvprintw(12,5,"Invalid choice. Press any key.");
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
    mvprintw(2, 5, "Your number : %d", res.my_num);      draw_big_number(res.my_num, 4, 5);
    mvprintw(2, 25, "Computer's number : %d", res.cpu_num); draw_big_number(res.cpu_num, 4,25);
    mvprintw(10, 5, "Your guess : %c", res.guess_num == 1 ? 'H' : 'L');
    mvprintw(12, 5, "Result : %s (%c%dG)    After your bank : %dG", res.win?"WIN":"LOSE", res.win?'+':'-', res.bet, res.new_money);
    mvprintw(16, 5, "Press any key");

    *money = res.new_money;

    // 5) 아무 키나 누르면 종료
    refresh(); getch();
}

void start_casino_game(int sock, int *user_money) {
    int choice = 0;
    while (1) {
        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "🎰 Welcome to the Casino 🎰");

        if (*user_money < 1) {
            mvprintw(3, 2, "You don't have enough money to play a game!");
            mvprintw(5, 2, "Press any key to return to the lobby...");
            refresh();
            getch();
            return;
        }

        mvprintw(3, 2, "Your Money: $%d", *user_money);
        mvprintw(5, 2, "[1] High & Low Game");
        mvprintw(6, 2, "[2] Blackjack Game");
        mvprintw(7, 2, "[3] Play Race Game");
        mvprintw(8, 2, "[4] View Rules");
        mvprintw(9, 2, "[5] Exit to Lobby");
        mvprintw(11, 2, "→ Enter your choice: ");
        char buf[8];
        echo();
        getstr(buf);
        noecho();
        refresh();

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
                mvprintw(1, 2, "[ Game Rules ]");
                mvprintw(3, 2, "- High & Low: Guess whether next number is higher or lower.");
                mvprintw(4, 2, "- Blackjack: Get closer to 21 than the dealer.");
                mvprintw(5, 2, "- Race: Pick a horse, and if it wins, you get rewards!");
                mvprintw(7, 2, "Press any key to return...");
                refresh();
                getch();
                break;
            case 5:
                return;
            default:
                mvprintw(13, 2, "Invalid choice. Press any key to try again.");
                refresh();
                getch();
                break;
        }
    }
}