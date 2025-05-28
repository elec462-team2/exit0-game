//client/casino.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <signal.h>
#include "../include/protocol.h"

static int max_y, max_x;

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
    { " *** ", "*   *", " ****", "    *", " *** " }
};

static void handle_resize(int sig) {
    endwin(); refresh(); clear();
    getmaxyx(stdscr, max_y, max_x);
}

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

static void render_result(const HighLowResponse *res) {
    clear(); box(stdscr, 0, 0);
    mvprintw(2, 5, "Your number : %d", res->my_num);      draw_big_number(res->my_num, 4, 5);
    mvprintw(2, 25, "Computer's number : %d", res->cpu_num); draw_big_number(res->cpu_num, 4,25);
    mvprintw(10, 5, "Your guess : %c", res->guess_num == 1 ? 'H' : 'L');
    mvprintw(12, 5, "Result : %s (%c%dG)    After your bank : %dG", res->win?"WIN":"LOSE", res->win?'+':'-', res->bet, res->new_money);
    mvprintw(16, 5, "Press any key");
    refresh();
}

void play_highlow_game(int sock, int *money) {
    initscr(); noecho(); cbreak(); curs_set(0);
    keypad(stdscr, TRUE);
    signal(SIGWINCH, handle_resize);
    getmaxyx(stdscr, max_y, max_x);

    echo();
    char buf[32];
    int bet = 0;
    char guess = 0;

    // 1) TUI로 배팅 입력
    clear(); box(stdscr,0,0);
    mvprintw(2,5,"Your bank : %dG", *money);
    mvprintw(4,5,"Enter bet : ");
    getstr(buf);
    bet = atoi(buf);
    if (bet <= 0 || bet > *money) {
        mvprintw(6,5,"Invalid bet. Press any key.");
        getch();
        endwin();
        return;
    }

    // 2) TUI로 H/L 입력
    mvprintw(6,5,"Guess [h] HIGH or [l] LOW: ");
    refresh();
    flushinp(); 
    guess = getch();
    noecho();
    if (guess != 'h' && guess != 'l') {
        mvprintw(8,5,"Invalid choice. Press any key.");
        getch();
        endwin();
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
        endwin();
        return;
    }
    render_result(&res);

    *money = res.new_money;

    // 5) 아무 키나 누르면 종료
    getch();

    endwin();
}

void start_casino_game(int sock, int *user_money) {
    while (1) {
        printf("\n[ Casino Main ]\n"
               "[1] High & Low Game\n"
               "[2] View Rules\n"
               "[3] Exit to Lobby\n"
               "→ choice: ");
        int m; scanf("%d",&m); getchar();

        if (m==2) {
            printf("Rule: guess higher or lower. win:+bet, lose:-bet\n");
            continue;
        }
        if (m==3) return;
        if (m==1)   play_highlow_game(sock, user_money);
        else        printf("Invalid.\n");
    }
}