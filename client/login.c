#include <ncursesw/ncurses.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include "../include/client_api.h"
#include "../include/protocol.h"

extern char global_user_id[MAX_ID_LEN];

void prompt_field(int y, const char *label, char *buf, size_t maxlen, int hide)
{
    int x = get_centered_x(label);  // 라벨 전체를 중앙정렬
    mvprintw(y, x, "%s", label);
    move(y, x + get_display_width(label));  // 정확한 커서 위치 이동

    if (hide) {
        noecho();
        int idx = 0, ch;
        while ((ch = getch()) != '\n' && idx < (int)maxlen - 1) {
            if (ch == KEY_BACKSPACE || ch == 127) {
                if (idx) {
                    idx--;
                    mvaddch(y, x + get_display_width(label) + idx, ' ');
                    move(y, x + get_display_width(label) + idx);
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

int perform_login(int sock, int *user_money)
{
    clear(); box(stdscr, 0, 0);
    int y = get_centered_y(8);
    mvprintw(y, get_centered_x("🔐  로그인"), "🔐  로그인");

    LoginRequest  req = { .cmd = CMD_LOGIN_REQ };
    LoginResponse res;

    prompt_field(y + 2, "ID        : ", req.user_id, MAX_ID_LEN, 0);
    prompt_field(y + 3, "Password  : ", req.password, MAX_PW_LEN, 1);

    send(sock, &req, sizeof(req), 0);
    ssize_t n = recv(sock, &res, sizeof(res), 0);
    if (n <= 0 || res.cmd != CMD_LOGIN_RES) return -1;

    clear(); box(stdscr, 0, 0);
    if (res.success) {
        *user_money = res.money;
        strcpy(global_user_id, req.user_id);
        mvprintw(get_centered_y(3), get_centered_x(res.message), "✅  %s", res.message);
        mvprintw(get_centered_y(3) + 2, get_centered_x("💰  잔고 : 0000 G"), "💰  잔고 : %d G", res.money);
        refresh(); getch();
        return 1;
    }
    mvprintw(get_centered_y(1), get_centered_x("❌  로그인에 실패했습니다."), "❌  로그인에 실패했습니다.");
    refresh(); getch();
    return 0;
}

int is_valid_id(const char *id) {
    size_t len = strlen(id);
    if (len < 4 || len > 10) return 0;
    for (size_t i = 0; i < len; i++) {
        if (!isalnum(id[i])) return 0;
    }
    return 1;
}

int perform_register(int sock)
{
    RegisterRequest  req = { .cmd = CMD_REGISTER_REQ };
    RegisterResponse res;
    int y;

    while (1) {
        clear(); box(stdscr, 0, 0);
        y = get_centered_y(8);
        mvprintw(y, get_centered_x("🆕  회원가입  (4~10 영문/숫자)"), "🆕  회원가입  (4~10 영문/숫자)");
        prompt_field(y + 2, "사용할 아이디 : ", req.user_id, MAX_ID_LEN, 0);

        if (!is_valid_id(req.user_id)) {
            mvprintw(y + 4, get_centered_x("❌  ID는 4~10자의 영문자 또는 숫자만 가능합니다."), "❌  ID는 4~10자의 영문자 또는 숫자만 가능합니다.");
            refresh(); getch(); continue;
        }

        memset(req.password, 0, sizeof(req.password));
        send(sock, &req, sizeof(req), 0);
        recv(sock, &res, sizeof(res), 0);

        if (res.success) {
            mvprintw(y + 4, get_centered_x(res.message), "✅  %s", res.message);
            refresh(); getch(); break;
        }
        mvprintw(y + 4, get_centered_x(res.message), "❌  %s", res.message);
        refresh(); getch();
    }

    prompt_field(y + 6, "비밀번호 설정 (6~12) : ", req.password, MAX_PW_LEN, 1);
    send(sock, &req, sizeof(req), 0);
    recv(sock, &res, sizeof(res), 0);

    clear(); box(stdscr, 0, 0);
    mvprintw(get_centered_y(1), get_centered_x(res.message), "%s", res.message);
    refresh(); getch();
    return res.success;
}
