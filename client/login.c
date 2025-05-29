#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../include/protocol.h"

/* 내부 도움 함수 */
static void prompt_field(int y, int x, const char *label,
                         char *buf, size_t maxlen, int hide)
{
    mvprintw(y, x, "%s", label);
    move(y, x + (int)strlen(label));
    if (hide) {
        /* 비밀번호 입력: 화면에 별표 출력 */
        noecho();
        int idx = 0, ch;
        while ((ch = getch()) != '\n' && idx < (int)maxlen - 1) {
            if (ch == KEY_BACKSPACE || ch == 127) {
                if (idx) {
                    idx--; mvaddch(y, x + (int)strlen(label) + idx, ' ');
                    move(y, x + (int)strlen(label) + idx);
                }
            } else {
                buf[idx++] = (char)ch;
                addch('*');
            }
        }
        buf[idx] = '\0';
        echo();
    } else {
        echo();
        getnstr(buf, (int)maxlen - 1);
    }
    clrtoeol();
}

int perform_login(int sock)
{
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "🔐  LOGIN");
    refresh();

    LoginRequest  req = { .cmd = CMD_LOGIN_REQ };
    LoginResponse res;

    prompt_field(3, 2, "ID        : ", req.user_id, MAX_ID_LEN, 0);
    prompt_field(4, 2, "Password  : ", req.password, MAX_PW_LEN, 1);

    send(sock, &req, sizeof(req), 0);

    ssize_t n = recv(sock, &res, sizeof(res), 0);
    if (n <= 0 || res.cmd != CMD_LOGIN_RES) return -1;

    clear(); box(stdscr, 0, 0);
    if (res.success) {
        mvprintw(2, 2, "✅  %s", res.message);
        mvprintw(4, 2, "💰  Balance : %d G", res.money);
        refresh();
        getch();
        return 1;
    }
    mvprintw(2, 2, "❌  Login failed.");
    refresh();
    getch();
    return 0;
}

int perform_register(int sock)
{
    RegisterRequest  req = { .cmd = CMD_REGISTER_REQ };
    RegisterResponse res;

    /* ------ ① ID 중복 확인 ------ */
    while (1) {
        clear(); box(stdscr, 0, 0);
        mvprintw(1, 2, "🆕  REGISTER  (4~10 영문/숫자)");
        prompt_field(3, 2, "New ID : ", req.user_id, MAX_ID_LEN, 0);

        memset(req.password, 0, sizeof(req.password));     /* 첫 요청엔 비번 X */
        send(sock, &req, sizeof(req), 0);
        recv(sock, &res, sizeof(res), 0);

        if (res.success) {
            mvprintw(5, 2, "✅  %s", res.message);
            refresh(); getch();
            break;
        }
        mvprintw(5, 2, "❌  %s", res.message);
        refresh(); getch();
    }

    /* ------ ② 최종 등록 ------ */
    prompt_field(5, 2, "Password (6~12) : ", req.password, MAX_PW_LEN, 1);
    send(sock, &req, sizeof(req), 0);
    recv(sock, &res, sizeof(res), 0);

    clear(); box(stdscr, 0, 0);
    mvprintw(2, 2, "%s", res.message);
    refresh(); getch();
    return res.success;
}
