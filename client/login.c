#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include "../include/protocol.h"

extern char global_user_id[MAX_ID_LEN];

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

int perform_login(int sock, int *user_money)
{
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "🔐  로그인");
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
        *user_money = res.money; // 서버에서 받은 돈 저장
        strcpy(global_user_id, req.user_id);  // ✅ ID 저장
        mvprintw(2, 2, "✅  %s", res.message);
        mvprintw(4, 2, "💰  잔고 : %d G", res.money);
        refresh();
        getch();
        return 1;
    }
    mvprintw(2, 2, "❌  로그인에 실패했습니다.");
    refresh();
    getch();
    return 0;
}

// 회원가입 요청한 아이디의 유효성 검사 : 영문.숫자로 이루어진 4-10자인지 확인합니다.
int is_valid_id(const char *id) {
    size_t len = strlen(id);
    if (len < 4 || len > 10) return 0;

    for (size_t i = 0; i < len; i++) {
        if (!isalnum(id[i])) return 0;  // 영문자/숫자가 아니면 거부
    }
    return 1;
}

int perform_register(int sock)
{
    RegisterRequest  req = { .cmd = CMD_REGISTER_REQ };
    RegisterResponse res;

    /* ------ ① ID 중복 확인 ------ */
    while (1) {
        clear(); box(stdscr, 0, 0);
        mvprintw(1, 2, "🆕  회원가입  (4~10 영문/숫자)");
        prompt_field(3, 2, "사용할 아이디 :", req.user_id, MAX_ID_LEN, 0);

        // 🔐 유효성 검사 먼저 수행
        if (!is_valid_id(req.user_id)) {
            mvprintw(5, 2, "❌  ID는 4~10자의 영문자 또는 숫자만 가능합니다.");
            refresh(); getch();
            continue;  // 다시 입력
        }
        
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
    prompt_field(5, 2, "비밀번호 설정 (6~12) : ", req.password, MAX_PW_LEN, 1);
    send(sock, &req, sizeof(req), 0);
    recv(sock, &res, sizeof(res), 0);

    clear(); box(stdscr, 0, 0);
    mvprintw(2, 2, "%s", res.message);
    refresh(); getch();
    return res.success;
}