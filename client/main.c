//client/main.c
#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <locale.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

extern void start_casino_game(int sock, int *user_money);
extern void start_labor_game(int sock, int *user_money);
extern void enter_chat_menu(int sock);
extern void draw_title_screen();  // 새로 추가한 타이틀 화면 함수
void show_ranking();
char global_user_id[MAX_ID_LEN] = {0};



/* ====== TUI 초기/종료 ====== */
static void init_ui(void)
{
    setlocale(LC_ALL,"");
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

static void show_welcome_screen(void)
{
    clear();
    box(stdscr, 0, 0);

    // 배경 도트 아트 (서부 느낌)
    mvprintw(3, 15, "              🌵                  🌅      🌵");
    mvprintw(4, 15, "         🌵        🏜️      ⛰️       🏜️            🌵");
    mvprintw(5, 15, "  ~~~    __||__        ~~~~~      __||__    ~~~");
    mvprintw(6, 15, "        _/    \\_                 _/    \\_");
    mvprintw(7, 15, "       |        |     🌞        |        |");
    mvprintw(8, 15, "       |  🐎    |               |    💼  |");
    mvprintw(9, 15, "       |________|               |________|");

    // 게임 타이틀
    attron(A_BOLD);
    mvprintw(11, 24, "💰 『 한탕의 꿈 』 💸");
    attroff(A_BOLD);

    // 부제 및 설명
    mvprintw(13, 15, "───────────────────────────────────────────────");
    mvprintw(14, 13, "노동장과 도박장을 전전하며 알바비를 불리는 인생 시뮬레이터.");
    mvprintw(15, 13, "기회는 단 한 번! 잃을 건 내 돈, 얻을 건 환상 뿐이라면?");
    mvprintw(16, 15, "───────────────────────────────────────────────");

    // 안내 문구
    attron(COLOR_PAIR(2));
    mvprintw(18, 15, "※ 본 게임은 터미널 창 크기 100x30 이상을 권장합니다.");
    mvprintw(19, 15, "* 게임 중 임의로 창을 조절하지 마세요. *");
    mvprintw(22, 16, "⏎ 시작하려면 <ENTER> | 종료하려면 [Ctrl+C] 또는 'q'를 입력하세요.");
    attroff(COLOR_PAIR(2));
    refresh();

    // 입력 한 번만 받고 즉시 분기
    int ch = getch();
    if (ch == '\n' || ch == KEY_ENTER) {
        return;  // 다음 흐름 진행 (메뉴 등)
    } else if (ch == 'q' || ch == 'Q') {
        cleanup_ui();
        exit(0);
    }
}

static int show_main_menu(void)
{
    char input[10];
    while (1) {
        clear(); box(stdscr, 0, 0);
        mvprintw(3, 4, "🧾  회원가입 or 로그인을 진행하세요 ");
        mvprintw(5, 6, "[1]  회원가입");
        mvprintw(6, 6, "[2]  로그인");
        mvprintw(8, 4, "선택지를 입력하세요 : ");
        refresh();

        echo();
        getstr(input);
        noecho();

        if (strlen(input) == 1 && (input[0] == '1' || input[0] == '2'))
            return input[0] - '0';

        mvprintw(10, 4, "잘못된 입력입니다. * 아무 키나 누르세요 * ");
        refresh(); getch();
    }
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
        mvprintw(3, 4, "❌  로그인 실패 : 아이디 혹은 비밀번호를 확인하세요. ");
        mvprintw(5, 4, "[1] 재시도   * 아무 키나 누르면 나갑니다 *");
        mvprintw(7, 4, "선택지를 입력하세요: ");
        
        refresh();

        char input[10];
        echo();
        getstr(input);
        noecho();

        if (strlen(input) == 1 && input[0] == '1') continue;  // 1 → 재시도
        return 0;  // 그 외 → 나가기
    }
}

/* ====== 클라이언트 실행 ====== */
// run_client() 위에 함수 선언 추가
static int show_post_login_menu(void) {
    clear(); box(stdscr, 0, 0);
    mvprintw(2, 4, "🎯 메인 메뉴: 무엇을 하시겠어요?");
    mvprintw(4, 6, "[1] 카지노 입장");
    mvprintw(5, 6, "[2] 노동장 입장");
    mvprintw(6, 6, "[3] 랭킹 보기");
    mvprintw(7, 6, "[4] 메신저");
    mvprintw(8, 6, "[Q] 로그아웃 후 종료"); 
    mvprintw(10, 4, "선택지를 입력하세요: ");
    refresh();

    char input[10];
    echo();
    getstr(input);
    noecho();

    if (strlen(input) == 1) {
        if (input[0] >= '1' && input[0] <= '4')
            return input[0] - '0';
        if (input[0] == 'q' || input[0] == 'Q')
            return -1;
    }

    // 잘못된 입력 처리
    mvprintw(12, 4, "잘못된 입력입니다. * 아무 키나 누르세요 *");
    refresh();
    getch();
    

    return 0;
}

// run_client 함수
void run_client(const char *ip, int port) {
    int sock = connect_to_server(ip, port);
    int user_money = 0;
    if (sock < 0) {
        cleanup_ui();
        fprintf(stderr, "서버에 연결하지 못했습니다.\n");
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
        if (choice == -1) {  // q 입력 처리
            AssetUpdateRequest req = { .cmd = CMD_UPDATE_ASSET };
            strcpy(req.user_id, global_user_id);
            req.money = user_money;
            send(sock, &req, sizeof(req), 0);

            mvprintw(12, 4, "또 봐요, 안녕!");
            refresh();
            getch();
            break;
        }

        switch (choice) {
            case 1:
                start_casino_game(sock, &user_money);
                break;
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


int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
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