#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

extern void start_burger_game(int *money, int sock);
char global_user_id[MAX_ID_LEN] = {0};



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
static int login_loop(int sock, int *user_money)
{
    while (1) {
        int ok = perform_login(sock, user_money);
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
// run_client() 위에 함수 선언 추가
static int show_post_login_menu(void) {
    clear(); box(stdscr, 0, 0);
    mvprintw(2, 4, "🎯  What would you like to do?");
    mvprintw(4, 6, "[1] Enter Casino");
    mvprintw(5, 6, "[2] Enter Work Zone");
    mvprintw(6, 6, "[3] View Ranking");
    mvprintw(7, 6, "[4] Chat");
    mvprintw(9, 4, "Select (1-4): ");
    refresh();

    int ch;
    while ((ch = getch())) {
        if (ch >= '1' && ch <= '4') return ch - '0';
    }

    return 0;
}

// 노동장 진입 후 노동 선택
static int show_work_menu(void) {
    clear(); box(stdscr, 0, 0);
    mvprintw(2, 4, "Welcome to the Work Zone!");
    mvprintw(4, 6, "[1] Burger Shop Part-time Job");
    mvprintw(5, 6, "[2] Package Sorting Part-time Job");
    mvprintw(7, 4, "Select (1-2): ");
    refresh();

    int ch;
    while ((ch = getch())) {
        if (ch == '1' || ch == '2') return ch - '0';
    }

    return 0;
}

// run_client 함수
void run_client(const char *ip, int port) {
    int sock = connect_to_server(ip, port);
    int user_money = 0;
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

    if (!login_loop(sock, &user_money)) {
        close(sock); cleanup_ui(); exit(1);
    }

    while (1) {
        
        // 로그인 성공 후 메뉴 출력
        choice = show_post_login_menu();

        switch (choice) {
            case 1:
                mvprintw(12, 4, "Entering Casino... (TODO)");
                refresh(); getch();
                break;
            case 2:
                {
                    // 노동장 -> 노동 방법 선택 
                    int work_choice = show_work_menu();
                    switch (work_choice) {
                        case 1:
                            start_burger_game(&user_money, sock);
                            break;
                        case 2:
                            start_package_game(&user_money, sock);
                            break;
                    }
                }
                break;
            case 3:
                mvprintw(12, 4, "Viewing Ranking... (TODO)");
                refresh(); getch();
                break;
            case 4:
                mvprintw(12, 4, "Starting Chat... (TODO)");
                refresh(); getch();
                break;
        }
    }

    close(sock);
}


int main(int argc, char *argv[])
{
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
