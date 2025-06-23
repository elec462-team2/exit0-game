#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <wchar.h>
#include <locale.h>
#include <signal.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

extern void start_casino_game(int sock, int *user_money);
extern void start_labor_game(int sock, int *user_money);
extern void enter_chat_menu(int sock);
extern int perform_register(int sock);
extern int perform_login(int sock, int *user_money);
extern void show_ranking();

char global_user_id[MAX_ID_LEN] = {0};

#define MENU_COUNT 2
const char *title_menu[] = {"게임 시작하기", "나가기"};

void handle_sigint(int sig) {
}

int get_display_width(const char *str) {
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));
    wchar_t wc;
    size_t len;
    int width = 0;

    const char *p = str;
    while ((len = mbrtowc(&wc, p, MB_CUR_MAX, &ps)) > 0) {
        int w = wcwidth(wc);
        width += (w > 0 ? w : 1);
        p += len;
    }
    return width;
}

void init_ui(void) {
    setlocale(LC_ALL, "");
    system("printf '\033[8;25;100t'");  // 터미널 크기 설정
    sleep(1);
    initscr(); cbreak(); noecho(); keypad(stdscr, TRUE); curs_set(0);
    start_color(); init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_RED, COLOR_BLACK);
}

void cleanup_ui(void) {
    endwin();
}

int get_centered_y(int lines) {
    int row, col;
    getmaxyx(stdscr, row, col);
    (void)col;
    return (row - lines) / 2;
}

int get_centered_x(const char *text) {
    int col = getmaxx(stdscr);
    mbstate_t ps; memset(&ps, 0, sizeof(ps));
    const char *p = text;
    wchar_t wc; size_t len; int width = 0;
    while ((len = mbrtowc(&wc, p, MB_CUR_MAX, &ps)) > 0) {
        int w = wcwidth(wc);
        width += (w > 0 ? w : 1);
        p += len;
    }
    return (col - width) / 2;
}

int connect_to_server(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = inet_addr(ip);
    if (connect(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        close(sock); return -1;
    }
    return sock;
}

void draw_title(void) {
    const char *title[] = {
        "    ██╗    █╗   ██████║ █╗       ███╗    █╗      █████████╗   ",
        " ████████║ █║   █╗      █║      █████╗   █║         █╗   █║   ",
        "           █║   █║      █║    ██╗    ██╗ █║         █║   █║   ",
        "  ██████╗  ███║ █████║  ███║  ██║    ██║ █║         █║   █║   ",
        "  █╗   █║  █║   █╗      █║    ██║    ██║ █║        █║    █║   ",
        "  █║   █║  █║   █║      █║      █████║   █║    █████████████║ ",
        "  ██████║  █║   ██████║ █║       ███║    █║           █╗      ",
        "           █║                            █║           █║      ",
        "   █╗      █║    ███████╗     ████████████║       ████████╗   ",
        "   █║           █╗      █╗               █║       █╗     █║   ",
        "   █║           █║      █║               █║       █║     █║   ",
        "   █████████║    ███████║                █║       ████████║   "
    };
    int col = getmaxx(stdscr);
    int title_lines = sizeof(title) / sizeof(title[0]);
    for (int i = 0; i < title_lines; i++) {
        int width = get_display_width(title[i]);
        int x = (col - width) / 2;
        if (x < 0) x = 0;
        mvprintw(i + 2, x, "%s", title[i]);
    }
}

int show_tui_main_menu(void) {
    int highlight = 0;
    int menu_y = 17;
    int col = getmaxx(stdscr);
    while (1) {
        clear(); box(stdscr, 0, 0); draw_title();
        for (int i = 0; i < MENU_COUNT; i++) {
            if (i == highlight) {
                attron(COLOR_PAIR(5) | A_BOLD);
                mvprintw(menu_y + i, (col - 20) / 2, "➤ %s", title_menu[i]);
                attroff(COLOR_PAIR(5) | A_BOLD);
            } else {
                mvprintw(menu_y + i, (col - 18) / 2, "  %s", title_menu[i]);
            }
        }
        refresh();
        int ch = getch();
        if (ch == KEY_UP) highlight = (highlight - 1 + MENU_COUNT) % MENU_COUNT;
        else if (ch == KEY_DOWN) highlight = (highlight + 1) % MENU_COUNT;
        else if (ch == '\n') return highlight;
    }
}

int show_login_menu(void) {
    const char *options[] = {"회원가입", "로그인"};
    int highlight = 0;
    while (1) {
        clear(); box(stdscr, 0, 0);
        int y = get_centered_y(6);
        mvprintw(y, get_centered_x("🧾  회원가입 or 로그인을 진행하세요"), "🧾  회원가입 or 로그인을 진행하세요");
        for (int i = 0; i < 2; i++) {
            int x = get_centered_x(options[i]);
            if (i == highlight) {
                attron(COLOR_PAIR(6) | A_BOLD);
                mvprintw(y + 2 + i, x - 2, "➤ %s", options[i]);
                attroff(COLOR_PAIR(6) | A_BOLD);
            } else {
                mvprintw(y + 2 + i, x, "%s", options[i]);
            }
        }
        refresh();
        int ch = getch();
        if (ch == KEY_UP) highlight = (highlight - 1 + 2) % 2;
        else if (ch == KEY_DOWN) highlight = (highlight + 1) % 2;
        else if (ch == '\n') return highlight + 1;
    }
}

static int login_loop(int sock, int *user_money) {
    while (1) {
        int ok = perform_login(sock, user_money);
        if (ok == 1) return 1;
        if (ok == -1) return -1;

        const char *options[] = {"재시도", "나가기"};
        int highlight = 0;
        int y = get_centered_y(7);

        while (1) {
            clear(); box(stdscr, 0, 0);
            mvprintw(y, get_centered_x("❌ 로그인 실패 : 아이디 혹은 비밀번호를 확인하세요."),
                     "❌ 로그인 실패 : 아이디 혹은 비밀번호를 확인하세요.");

            for (int i = 0; i < 2; i++) {
                int x = get_centered_x(options[i]);
                if (i == highlight) {
                    attron(COLOR_PAIR(6) | A_BOLD);
                    mvprintw(y + 2 + i, x - 2, "➤ %s", options[i]);
                    attroff(COLOR_PAIR(6) | A_BOLD);
                } else {
                    mvprintw(y + 2 + i, x, "%s", options[i]);
                }
            }
            refresh();

            int ch = getch();
            if (ch == KEY_UP) highlight = (highlight - 1 + 2) % 2;
            else if (ch == KEY_DOWN) highlight = (highlight + 1) % 2;
            else if (ch == '\n') break;
        }

        if (highlight == 0) continue; 
        else return 0;                
    }
}

int show_post_login_menu(void) {
    const char *options[] = {"카지노 입장", "노동장 입장", "랭킹 보기", "메신저", "로그아웃 후 종료"};
    int highlight = 0;
    while (1) {
        clear(); box(stdscr, 0, 0);
        draw_title();
        int y = get_centered_y(8) + 7;
        mvprintw(y, get_centered_x("🎯 메인 메뉴: 무엇을 하시겠어요?"), "🎯 메인 메뉴: 무엇을 하시겠어요?");
        for (int i = 0; i < 5; i++) {
            int x = get_centered_x(options[i]);
            if (i == highlight) {
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(y + 2 + i, x - 2, "➤ %s", options[i]);
                attroff(COLOR_PAIR(3) | A_BOLD);
            } else {
                mvprintw(y + 2 + i, x, "%s", options[i]);
            }
        }
        refresh();
        int ch = getch();
        if (ch == KEY_UP) highlight = (highlight - 1 + 5) % 5;
        else if (ch == KEY_DOWN) highlight = (highlight + 1) % 5;
        else if (ch == '\n') return (highlight == 4) ? -1 : highlight + 1;
    }
}

void run_client(const char *ip, int port) {
    int sock = connect_to_server(ip, port);
    if (sock < 0) {
        clear(); box(stdscr, 0, 0);
        mvprintw(get_centered_y(1), get_centered_x("서버에 연결하지 못했습니다."), "서버에 연결하지 못했습니다.");
        refresh(); getch();
        cleanup_ui(); exit(1);
    }

    int choice = show_tui_main_menu();
    if (choice == 1) {
        clear(); box(stdscr, 0, 0);
        mvprintw(get_centered_y(1), get_centered_x("게임을 종료합니다."), "게임을 종료합니다.");
        refresh(); getch();
        cleanup_ui(); exit(0);
    }

    choice = show_login_menu();
    int user_money = 0;
    if (choice == 1) {
        if (!perform_register(sock)) {
            clear(); box(stdscr, 0, 0);
            mvprintw(get_centered_y(1), get_centered_x("회원가입에 실패했습니다."), "회원가입에 실패했습니다.");
            refresh(); getch();
            close(sock); cleanup_ui(); exit(1);
        }
    }
    if (!login_loop(sock, &user_money)) {
        clear(); box(stdscr, 0, 0);
        mvprintw(get_centered_y(1), get_centered_x("로그인 프로세스를 종료합니다."), "로그인 프로세스를 종료합니다.");
        refresh(); getch();
        close(sock); cleanup_ui(); exit(1);
    }

    while (1) {
        choice = show_post_login_menu();
        if (choice == -1) {
            AssetUpdateRequest req = { .cmd = CMD_UPDATE_ASSET };
            strcpy(req.user_id, global_user_id);
            req.money = user_money;
            send(sock, &req, sizeof(req), 0);
            clear(); box(stdscr, 0, 0);
            mvprintw(get_centered_y(1), get_centered_x("또 봐요, 안녕!"), "또 봐요, 안녕!");
            refresh(); getch();
            break;
        }
        switch (choice) {
            case 1: start_casino_game(sock, &user_money); break;
            case 2:
                start_labor_game(sock, &user_money);
                break;
            case 3:
                show_ranking();
                break;
            case 4:
                enter_chat_menu(sock);
                break;
        }
    }
    close(sock);
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    signal(SIGINT, SIG_IGN);

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }
    srand(time(NULL));
    init_ui();
    run_client(argv[1], atoi(argv[2]));
    cleanup_ui();
    return 0;
}
