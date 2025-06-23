//client/casino.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <locale.h>
#include <signal.h>
#include "../include/client_api.h"
#include "../include/protocol.h"

const char *big_digits[10][5] = {
    { " ███ ", "█   █", "█   █", "█   █", " ███ " },
    { "  █  ", " ██  ", "  █  ", "  █  ", " ███ " },
    { " ███ ", "    █", " ███ ", "█    ", "█████" },
    { "████ ", "    █", " ███ ", "    █", "████ " },
    { "█  █ ", "█  █ ", "█████", "   █ ", "   █ " },    
    { "█████", "█    ", "████ ", "    █", "████ " },
    { " ███ ", "█    ", "████ ", "█   █", " ███ " },
    { "█████", "   █ ", "  █  ", " █   ", " █   " },
    { " ███ ", "█   █", " ███ ", "█   █", " ███ " },
    { " ███ ", "█   █", " ████", "    █", " ███ " },
};

void draw_big_number(int num, int row, int col) {
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
    int y = get_centered_y(12);
    mvprintw(y, get_centered_x("🏇 자자 곧 시합 시작해요 빨리 배팅하세요! 경마 게임에 온 걸 환영해요🏇"),
             "🏇 자자 곧 시합 시작해요 빨리 배팅하세요! 경마 게임에 온 걸 환영해요🏇");
    mvprintw(y + 2, get_centered_x("말을 고르고 베팅 금액을 입력하세요."), "말을 고르고 베팅 금액을 입력하세요.");
    mvprintw(y + 4, get_centered_x("잔고: [0000G]"), "잔고: [%dG]", *money);

    mvprintw(y + 4, getmaxx(stdscr) - 25, "[승률 & 배당률]");
    mvprintw(y + 6, getmaxx(stdscr) - 25, "🐎 : 66.7%% | x1.5");
    mvprintw(y + 7, getmaxx(stdscr) - 25, "🎠 :   40%% | x2.5");
    mvprintw(y + 8, getmaxx(stdscr) - 25, "🐪 :   20%% | x5");

    mvprintw(y + 6, get_centered_x("당신의 말을 고르세요 ( 0: 🐎 1: 🎠 2: 🐪 ) → ")-3,
             "당신의 말을 고르세요 ( 0: 🐎 1: 🎠 2: 🐪 ) → ");
    getnstr(buf, sizeof(buf) - 1);
    if (strlen(buf) != 1 || buf[0] < '0' || buf[0] > '2') {
        mvprintw(y + 8, get_centered_x("그런 말은 선택지에 없습니다. * 아무 키나 누르세요... *"),
                 "그런 말은 선택지에 없습니다. * 아무 키나 누르세요... *");
        getch(); return;
    }
    choice = buf[0] - '0';

    mvprintw(y + 8, get_centered_x("베팅 금액을 입력하세요: ")-3, "베팅 금액을 입력하세요: ");
    getnstr(buf, sizeof(buf) - 1);
    bet = atoi(buf);
    if (bet <= 0 || bet > *money) {
        mvprintw(y + 10, get_centered_x("잘못된 금액이거나 당신의 잔고를 초과한 금액입니다. * 아무 키나 누르세요... *"),
                 "잘못된 금액이거나 당신의 잔고를 초과한 금액입니다. * 아무 키나 누르세요... *");
        getch(); return;
    }

    noecho();
    CommandType cmd = CMD_RACE_REQ;
    send(sock, &cmd, sizeof(cmd), 0);
    RaceRequest req = { .cmd = CMD_RACE_REQ, .selected_horse = choice, .bet = bet };
    send(sock, &req, sizeof(req), 0);

    const char *horse_names[3] = {"🐎", "🎠", "🐪"};
    int finish_line = 50;
    int screen_mid = getmaxx(stdscr) / 2;
    int track_start = screen_mid - finish_line / 2;

    while (1) {
        RaceStepResponse step;
        ssize_t n = recv(sock, &step, sizeof(step), 0);
        if (n <= 0 || step.cmd != CMD_RACE_STEP) break;

        clear(); box(stdscr, 0, 0);
        for (int i = 0; i < 3; i++) {
            int x = step.horse_positions[i];
            if (x > finish_line) x = finish_line;
            mvprintw(2 + i * 3, track_start + x, "%s", horse_names[i]);
            mvprintw(2 + i * 3 + 1, track_start + finish_line + 1, "| END");
            mvhline(2 + i * 3 + 2, track_start, '-', finish_line + 6);
        }

        refresh();
        if (step.finished) break;
    }

    RaceResultResponse res;
    recv(sock, &res, sizeof(res), 0);

    clear(); box(stdscr, 0, 0);
    int ry = get_centered_y(8);
    mvprintw(ry, get_centered_x("🏁 경기 종료!"), "🏁 경기 종료!");
    mvprintw(ry + 2, get_centered_x("당신이 선택한 말: 🐎"), "당신이 선택한 말: %s (말 %d)", horse_names[res.user_choice], res.user_choice);
    mvprintw(ry + 3, get_centered_x("승리한 말: 🐎"), "승리한 말: %s (말 %d)", horse_names[res.winner], res.winner);
    mvprintw(ry + 4, get_centered_x("배당률: 000G"), "배당률: %dG", res.payout);
    mvprintw(ry + 5, get_centered_x("경기 종료 후 잔고: 0000G"), "경기 종료 후 잔고: %dG", res.new_money);
    *money = res.new_money;

    mvprintw(ry + 7, get_centered_x("* 계속하려면 아무 키나 누르세요... *"), "* 계속하려면 아무 키나 누르세요... *");
    refresh(); getch();
}

void play_blackjack_game(int sock, int *money) {
    char buf[16];
    int bet = 0;

    echo(); clear(); box(stdscr, 0, 0);
    int y = get_centered_y(10);
    mvprintw(y, get_centered_x("🃏 블랙잭 테이블에 온 걸 환영해! 🃏"), "🃏 블랙잭 테이블에 온 걸 환영해! 🃏");
    mvprintw(y + 2, get_centered_x("규칙은 알지? 총합이 21을 넘지 않고 더 높은 사람이 승자야!"), "규칙은 알지? 총합이 21을 넘지 않고 더 높은 사람이 승자야!");
    mvprintw(y + 4, get_centered_x("잔고 : [0000G]"), "잔고 : [%dG]", *money);
    mvprintw(y + 6, get_centered_x("베팅 금액을 입력하세요 : "), "베팅 금액을 입력하세요 : ");
    getstr(buf); bet = atoi(buf);
    noecho();

    if (bet <= 0 || bet > *money) {
        mvprintw(y + 8, get_centered_x("잘못된 금액입니다. * 아무 키나 누르세요... *"), "잘못된 금액입니다. * 아무 키나 누르세요... *");
        getch(); return;
    }

    CommandType cmd = CMD_BLACKJACK_REQ;
    send(sock, &cmd, sizeof(cmd), 0);
    BlackjackRequest req = { .cmd = CMD_BLACKJACK_REQ, .bet = bet };
    send(sock, &req, sizeof(req), 0);

    BlackjackResponse res;
    recv(sock, &res, sizeof(res), 0);

    while (1) {
        clear(); box(stdscr, 0, 0);
        mvprintw(y, get_centered_x("방금 뽑은 카드"), "방금 뽑은 카드");
        draw_big_number(res.last_card, y + 2, get_centered_x("     "));
        mvprintw(y + 8, get_centered_x("총합: 00"), "총합: %d", res.player_score);
        mvprintw(y + 10, get_centered_x("[h] 한 장 더!  [s] 스톱!!!"), "[h] 한 장 더!  [s] 스톱!!!");
        refresh();

        char choice = getch();
        if (choice == 'h') {
            cmd = CMD_BLACKJACK_HIT;
            send(sock, &cmd, sizeof(cmd), 0);
            recv(sock, &res, sizeof(res), 0);
            if (res.is_final) break;
        } else if (choice == 's') break;
    }

    cmd = CMD_BLACKJACK_RESULT;
    send(sock, &cmd, sizeof(cmd), 0);
    recv(sock, &res, sizeof(res), 0);
    *money = res.new_money;

    clear(); box(stdscr, 0, 0);
    int screen_mid = getmaxx(stdscr) / 2;
    y = 4;
    mvprintw(y, get_centered_x("🔢 결과 발표 🔢"), "🔢 결과 발표 🔢");

    mvprintw(y + 2, screen_mid - 20, "[ YOU ]");
    draw_big_number(res.player_score, y + 4, screen_mid - 20);
    mvprintw(y + 10, screen_mid - 20, "너의 총합: %d", res.player_score);

    mvprintw(y + 2, screen_mid + 10, "[ DEALER ]");
    draw_big_number(res.dealer_score, y + 4, screen_mid + 10);
    mvprintw(y + 10, screen_mid + 10, "딜러의 총합: %d", res.dealer_score);

    const char *reason_msg = (res.player_score > 21) ? "뭐야 21을 넘겼구나! 소고기 맛있게 먹을게~!" :
                             (res.dealer_score > 21) ? "... 내가 21을 넘겼네. 흐름 탔으니까 한 판 더 해야지?" :
                             (res.player_score == res.dealer_score) ? "숫자가 똑같네? 재미없게" :
                             (res.player_score > res.dealer_score) ? "나보다 숫자가 높네? 너가 이겼어.." :
                             "점수 차 보이지? 사장님보다 너가 돈을 더 챙겨주네 ㅎㅎ";

    mvprintw(y + 13, get_centered_x(reason_msg), "%s", reason_msg);
    mvprintw(y + 14, get_centered_x("(결과: 승/무/패)"), "(결과: %s)", res.win == 1 ? "승리" : (res.win == 2 ? "무승부" : "패배"));
    mvprintw(y + 16, get_centered_x("최종 자산: 0000000G"), "최종 자산: %dG", *money);
    mvprintw(y + 18, get_centered_x("* 계속하려면 아무 키나 누르세요... *"), "* 계속하려면 아무 키나 누르세요... *");
    refresh(); getch();
}

void play_highlow_game(int sock, int *money) {
    char buf[32];
    int bet = 0;
    int highlight = 0;
    const char *guess_options[] = {"높을 것이다 (H)", "낮을 것이다 (L)"};

    echo(); clear(); box(stdscr, 0, 0);
    int y = get_centered_y(10);
    mvprintw(y, get_centered_x("🔢 하이 앤 로우 테이블에 온 걸 환영해! 🔢"), "🔢 하이 앤 로우 테이블에 온 걸 환영해! 🔢");
    mvprintw(y + 2, get_centered_x("내가 숫자를 줄 테니 나보다 높을지 낮을지 맞혀 봐~"), "내가 숫자를 줄 테니 나보다 높을지 낮을지 맞혀 봐~");
    mvprintw(y + 4, get_centered_x("잔고 : [0000G]"), "잔고 : [%dG]", *money);
    mvprintw(y + 6, get_centered_x("베팅 금액을 입력하세요 : "), "베팅 금액을 입력하세요 : ");
    getstr(buf);
    bet = atoi(buf);
    if (bet <= 0 || bet > *money) {
        mvprintw(y + 8, get_centered_x("잘못된 금액입니다. * 아무 키나 누르세요... *"), "잘못된 금액입니다. * 아무 키나 누르세요... *");
        getch(); return;
    }
    noecho();

    while (1) {
        clear(); box(stdscr, 0, 0);
        mvprintw(y, get_centered_x("🔢 하이 앤 로우 선택"), "🔢 하이 앤 로우 선택");
        mvprintw(y + 2, get_centered_x("어떻게 될 것 같아?"), "어떻게 될 것 같아?");
        for (int i = 0; i < 2; i++) {
            int x = get_centered_x(guess_options[i]);
            if (i == highlight) {
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(y + 4 + i, x - 2, "➤ %s", guess_options[i]);
                attroff(COLOR_PAIR(3) | A_BOLD);
            } else {
                mvprintw(y + 4 + i, x, "%s", guess_options[i]);
            }
        }
        refresh();
        int ch = getch();
        if (ch == KEY_UP) highlight = (highlight - 1 + 2) % 2;
        else if (ch == KEY_DOWN) highlight = (highlight + 1) % 2;
        else if (ch == '\n') break;
    }

    int guess_num = (highlight == 0) ? 1 : 0;

    CommandType hcmd = CMD_HIGHLOW_REQ;
    send(sock, &hcmd, sizeof(hcmd), 0);

    HighLowRequest req = {
        .cmd = CMD_HIGHLOW_REQ,
        .bet = bet,
        .guess_num = guess_num
    };
    send(sock, &req, sizeof(req), 0);

    HighLowResponse res;
    int n = recv(sock, &res, sizeof(res), 0);
    if (n <= 0 || res.cmd != CMD_HIGHLOW_RES) return;

    *money = res.new_money;

    clear(); box(stdscr, 0, 0);
    int row = get_centered_y(12);
    int screen_mid = getmaxx(stdscr) / 2;

    mvprintw(row, get_centered_x("🔢 결과 발표 🔢"), "🔢 결과 발표 🔢");
    mvprintw(row + 2, screen_mid - 20, "[ YOU ]");
    draw_big_number(res.my_num, row + 3, screen_mid - 20);
    mvprintw(row + 2, screen_mid + 10, "[ DEALER ]");
    draw_big_number(res.cpu_num, row + 3, screen_mid + 10);

    mvprintw(row + 9, get_centered_x("너가 고른 선택 : 높음 (H)"), "너가 고른 선택 : %s", guess_num == 1 ? "높음 (H)" : "낮음 (L)");

    char result_msg[128];
    sprintf(result_msg, "결과 : %s (%c%dG)    최종 자산 : %dG",
            res.win ? "운이 좋네 네가 이겼어!" : "돈은 내가 잠시 맡아둘게 ㅋㅋ",
            res.win ? '+' : '-', res.bet, res.new_money);
    mvprintw(row + 11, get_centered_x(result_msg), "%s", result_msg);

    mvprintw(row + 13, get_centered_x("* 아무 키나 누르세요... *"), "* 아무 키나 누르세요... *");
    refresh(); getch();
}

void start_casino_game(int sock, int *user_money) {
    const char *options[] = {"하이 앤 로우 게임", "블랙잭 게임", "경마 게임", "게임 규칙 설명", "로비로 나가기"};
    int highlight = 0;
    while (1) {
        clear(); box(stdscr, 0, 0);
        int y = get_centered_y(10);
        mvprintw(y, get_centered_x("🎰 카지노에 온 것을 환영합니다 🎰"), "🎰 카지노에 온 것을 환영합니다 🎰");

        if (*user_money < 1) {
            mvprintw(y + 2, get_centered_x("이런, 도박할 돈이 없네요. 노동장에서 돈을 벌어보세요."), "이런, 도박할 돈이 없네요. 노동장에서 돈을 벌어보세요.");
            mvprintw(y + 4, get_centered_x("* 로비로 돌아가려면 아무 키나 누르세요... *"), "* 로비로 돌아가려면 아무 키나 누르세요... *");
            refresh(); getch(); return;
        }

        mvprintw(y + 2, get_centered_x("잔고: 0000G"), "잔고: %dG", *user_money);
        for (int i = 0; i < 5; i++) {
            int x = get_centered_x(options[i]);
            if (i == highlight) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(y + 4 + i, x - 2, "➤ %s", options[i]);
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(y + 4 + i, x, "%s", options[i]);
            }
        }
        refresh();
        int ch = getch();
        if (ch == KEY_UP) highlight = (highlight - 1 + 5) % 5;
        else if (ch == KEY_DOWN) highlight = (highlight + 1) % 5;
        else if (ch == '\n') {
            switch (highlight) {
                case 0: play_highlow_game(sock, user_money); break;
                case 1: play_blackjack_game(sock, user_money); break;
                case 2: play_race_game(sock, user_money); break;
                case 3:
                    clear(); box(stdscr, 0, 0);
                    int yy = get_centered_y(20);
                    mvprintw(yy, get_centered_x("[ 🎮 게임 규칙 설명서 ]"), "[ 🎮 게임 규칙 설명서 ]");
                    mvprintw(yy + 2, 4, "① 하이⤴️  & 로우⤵️ (난이도: 하)");
                    mvprintw(yy + 3, 6, "- 당신의 숫자가 딜러의 숫자보다 클 지, 작을 지 맞히는 게임입니다.");
                    mvprintw(yy + 4, 6, "- 숫자는 1부터 100 사이에서 무작위로 생성됩니다.");
                    mvprintw(yy + 6, 4, "② 블랙잭 🃏 (난이도: 중)");
                    mvprintw(yy + 7, 6, "- 당신과 딜러가 카드를 뽑으며 21에 가까운 수를 만드는 게임입니다.");
                    mvprintw(yy + 8, 6, "- 점수가 21을 넘으면 즉시 패배합니다.");
                    mvprintw(yy + 9, 6, "- 딜러보다 높고 21 이하일 경우 승리하여 보상을 얻습니다.");
                    mvprintw(yy + 11, 4, "③ 경마 🐎 (난이도: 상)");
                    mvprintw(yy + 12, 6, "- 승률이 다른 3마리의 말 중 하나를 선택합니다.");
                    mvprintw(yy + 13, 6, "- 선택한 말이 1등으로 도착하면 배당률에 따라 보상을 받습니다.");
                    mvprintw(yy + 14, 6, "- 승률이 낮을 말일수록 이겼을 때의 보상이 큽니다.");
                    mvprintw(yy + 16, get_centered_x("* 아무 키나 눌러서 메인 메뉴로 돌아가세요... *"), "* 아무 키나 눌러서 메인 메뉴로 돌아가세요... *");
                    refresh(); getch(); break;
                case 4: return;
            }
        }
    }
}