#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

/* ====== TUI 초기/종료 ====== */
static void init_ui(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);          /* 커서 보이기 (입력 필드용) */
    start_color();
    init_pair(1, COLOR_CYAN,   COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
}

static void cleanup_ui(void)
{
    endwin();
}

/* ====== 네트워크 ====== */
static int connect_to_server(const char *ip, int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in sa = {0};
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons(port);
    sa.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        close(sock); return -1;
    }
    return sock;
}

/* ====== UI 화면 ====== */
static void show_welcome_screen(void)
{
    clear();
    box(stdscr, 0, 0);
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(3, 4, "🎮  Welcome to the Game !");
    attroff(A_BOLD);
    attron(COLOR_PAIR(2));
    mvprintw(5, 4, "Press <ENTER> to continue,  Ctrl+C / q  to quit.");
    attroff(COLOR_PAIR(2));
    refresh();
    int ch;
    while ((ch = getch()) != '\n' && ch != KEY_ENTER && ch != 'q');
    if (ch == 'q') {
        cleanup_ui();
        exit(0);
    }
}

static int show_main_menu(void)
{
    clear(); box(stdscr, 0, 0);
    mvprintw(3, 4, "🧾  MAIN  MENU");
    mvprintw(5, 6, "[1]  Register");
    mvprintw(6, 6, "[2]  Login");
    mvprintw(8, 4, "Select (1/2) : ");
    refresh();

    int ch;
    while ((ch = getch()) != '1' && ch != '2');
    return ch - '0';
}

/* ====== 로그인 루프 ====== */
static int login_loop(int sock)
{
    while (1) {
        int ok = perform_login(sock);
        if (ok == 1) return 1;
        if (ok == -1) return -1;  /* 통신 오류 */

        /* 실패 시 옵션 */
        clear(); box(stdscr, 0, 0);
        mvprintw(3, 4, "❌  Login failed.");
        mvprintw(5, 4, "[1] 재시도   [2] 나가기");
        refresh();

        int ch;
        while ((ch = getch()) != '1' && ch != '2');
        if (ch == '2') return 0;   /* 나가기 */
    }
}

/* ====== 클라이언트 실행 ====== */
void run_client(const char *ip, int port)
{
    int sock = connect_to_server(ip, port);
    if (sock < 0) {
        cleanup_ui();
        fprintf(stderr, "Cannot connect to server.\n");
        exit(1);
    }

    show_welcome_screen();

    int choice = show_main_menu();
    if (choice == 1) {                          /* Register */
        if (!perform_register(sock)) {
            close(sock); cleanup_ui(); exit(1);
        }
    }

    if (!login_loop(sock)) {
        close(sock); cleanup_ui(); exit(1);
    }

    clear(); box(stdscr, 0, 0);
    mvprintw(3, 4, "🚀  Login success!  (TODO: game / chat menu)");
    refresh(); getch();

    close(sock);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    init_ui();
    run_client(argv[1], atoi(argv[2]));
    cleanup_ui();
    return 0;
}
