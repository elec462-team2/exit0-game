#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <locale.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

void update_user_asset(const char *userid, int new_balance);
extern char global_user_id[MAX_ID_LEN];

#define MAX_INGREDIENTS 6
#define MAX_ORDER 7

const char *ingredients[MAX_INGREDIENTS] = {
    "빵", "양상추", "토마토", "패티", "치즈", "양파"
};


#define REGION_COUNT 18

extern char global_user_id[MAX_ID_LEN];

typedef struct {
    char region[20];
    char center;
} RegionMap;

RegionMap regions[REGION_COUNT] = {
    {"서울", 'A'}, {"인천", 'A'}, {"수원", 'A'}, {"고양", 'A'},
    {"부산", 'B'}, {"대구", 'B'}, {"경주", 'B'}, {"포항", 'B'},
    {"광주", 'C'}, {"전주", 'C'}, {"목포", 'C'}, {"군산", 'C'},
    {"춘천", 'D'}, {"원주", 'D'}, {"강릉", 'D'}, {"대전", 'D'},
    {"제주", 'E'}, {"서귀포", 'E'}
};

void show_region_mapping(int row, int col) {
    mvprintw(row, col,   "[A] 창고: 서울, 인천, 수원, 고양");
    mvprintw(row+1, col, "[B] 창고: 부산, 대구, 경주, 포항");
    mvprintw(row+2, col, "[C] 창고: 광주, 전주, 목포, 군산");
    mvprintw(row+3, col, "[D] 창고: 춘천, 원주, 강릉, 대전");
    mvprintw(row+4, col, "[E] 창고: 제주, 서귀포");
}

void start_burger_game(int *money, int sock) {
    extern char global_user_id[MAX_ID_LEN];
    int running = 1;

    while (running) {
        clear();
        mvprintw(1, 2, "<< 햄빌리버블 버거 가게 >>");
        mvprintw(3, 2, "버거 만들기 알바 하러 오셨죠? 앞치마 매고 이리오세요~");

        // 1. 게임 시작 요청
        CommandType cmd = CMD_BURGER_REQ;
        send(sock, &cmd, sizeof(cmd), 0);

        BurgerOrderRequest req = { .cmd = CMD_BURGER_REQ };
        strcpy(req.user_id, global_user_id);
        send(sock, &req, sizeof(req), 0);

        // 2. 주문서 수신
        BurgerOrderSheet sheet;
        recv(sock, &sheet, sizeof(sheet), 0);

        // 3. 주문서 출력
        const char *ingredients[] = {"빵", "양상추", "토마토", "패티", "치즈", "양파"};
        for (int i = 0; i < 6; i++)
            mvprintw(5 + i, 2, "[%d] %s", i + 1, ingredients[i]);

        mvprintw(13, 2, "오늘의 주문:");
        for (int i = 0; i < sheet.order_len; i++)
            mvprintw(15, 2 + i * 10, "[%s]", sheet.order[i]);

        mvprintw(17, 2, "순서에 맞게 재료를 입력하세요 (예시. 1 3 5 ...):");
        echo();
        char input_line[64] = {0};
        move(18, 2);
        getnstr(input_line, sizeof(input_line) - 1);
        noecho();

        if (tolower(input_line[0]) == 'q') {
            // 종료 신호 보내기 위해 같은 형식으로 전송
            cmd = CMD_BURGER_RES;
            send(sock, &cmd, sizeof(cmd), 0);

            BurgerAnswerRequest quit_req = { .cmd = CMD_BURGER_RES };
            strcpy(quit_req.user_id, global_user_id);
            strcpy(quit_req.input, "Q");  // 종료 신호로 "Q" 입력
            send(sock, &quit_req, sizeof(quit_req), 0);

            break;
        }

        // 4. 정답 제출
        cmd = CMD_BURGER_RES;
        send(sock, &cmd, sizeof(cmd), 0);

        BurgerAnswerRequest answer = { .cmd = CMD_BURGER_RES };
        strcpy(answer.user_id, global_user_id);
        strncpy(answer.input, input_line, sizeof(answer.input) - 1);
        send(sock, &answer, sizeof(answer), 0);

        // 5. 결과 수신
        BurgerResultResponse result;
        recv(sock, &result, sizeof(result), 0);

        if (result.correct)
            mvprintw(20, 2, "🍔 정답! +%d원 획득!", result.delta_money);
        else
            mvprintw(20, 2, "😭 오답! %d원 차감!", result.delta_money);

        *money = result.updated_money;
        mvprintw(22, 2, "💰 현재 자산: %d원", *money);
        mvprintw(24, 2, "계속하려면 Enter, 그만하려면 q 입력");
        refresh();

        char ch = getch();
        if (tolower(ch) == 'q') break;
    }
}

void start_package_game(int *money, int sock) {
    extern char global_user_id[MAX_ID_LEN];

    initscr();
    cbreak();
    noecho();
    curs_set(1);

    while (1) {
        // [1] 서버에 문제 요청
        PackageRequest req = {
            .cmd = CMD_PACKAGE_REQ
        };
        strcpy(req.user_id, global_user_id);
        send(sock, &req, sizeof(req), 0);

        // [2] 서버로부터 지역 수신
        PackageResponse resp;
        recv(sock, &resp, sizeof(resp), 0);

        clear(); box(stdscr, 0, 0);
        mvprintw(1, 2, "=== 📦 코팡 물류 센터 🚚 ===");
        mvprintw(3, 2, "어서와요, 물류 분류 정확하게 부탁해요. 틀리면 일급 날아가요!");
        show_region_mapping(5, 2);
        mvprintw(11, 2, "목적지: %s", resp.region_info.region);
        mvprintw(13, 2, "물류 창고 이름을 적으세요 (A~E), * 그만 두려면 [q]: ");
        refresh();

        move(14, 2);
        int ch = getch();
        char upper_ch = toupper(ch);

        if (tolower(ch) == 'q') {
            PackageAnswer quit = {
                .cmd = CMD_PACKAGE_RES,      // 그대로 유지
                .answer = 'Q'                // 대문자 Q로 보내기
            };
            strcpy(quit.user_id, global_user_id);
            send(sock, &quit, sizeof(quit), 0);

            mvprintw(16, 2, "종료합니다~");
            refresh();
            getch();

            // 🚨 핵심 수정 부분
            flushinp();     // 입력 버퍼 제거 (getch() 영향 제거)
            clear();        // 화면 초기화
            refresh();      // 초기화 적용

            break;
        }

        // [3] 사용자 입력 전송
        PackageAnswer answer = {
            .cmd = CMD_PACKAGE_RES,
            .answer = upper_ch
        };
        strcpy(answer.user_id, global_user_id);
        send(sock, &answer, sizeof(answer), 0);

        // [4] 결과 수신
        PackageResult result;
        recv(sock, &result, sizeof(result), 0);

        if (result.correct) {
            *money += result.delta_money;
            mvprintw(16, 2, "정답! +10");
        } else {
            *money += result.delta_money;
            mvprintw(16, 2, "오답! -5");
            mvprintw(17, 2, "정답은: %c", result.correct_answer);
        }

        mvprintw(19, 2, "잔고: %d", *money);
        mvprintw(21, 2, "* 다음 문제는 아무 키나 누르세요... *");
        refresh();
        getch();
    }

    curs_set(0);
    endwin();
}

int start_labor_game(int sock, int *money) {
    const char *options[] = {"햄버거 만들기 알바 시작하기", "택배 분류 알바 시작하기", "메인 메뉴로 나가기"};
    int highlight = 0;

    while (1) {
        clear(); box(stdscr, 0, 0);
        int y = get_centered_y(6);
        mvprintw(y, get_centered_x("노동장에 왔으면 성실하게 일을 해야 합니다."), "노동장에 왔으면 성실하게 일을 해야 합니다.");
        for (int i = 0; i < 3; i++) {
            int x = get_centered_x(options[i]);
            if (i == highlight) {
                attron(COLOR_PAIR(2) | A_BOLD);
                mvprintw(y + 2 + i, x - 2, "➤ %s", options[i]);
                attroff(COLOR_PAIR(2) | A_BOLD);
            } else {
                mvprintw(y + 2 + i, x, "%s", options[i]);
            }
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP) highlight = (highlight - 1 + 3) % 3;
        else if (ch == KEY_DOWN) highlight = (highlight + 1) % 3;
        else if (ch == '\n') {
            switch (highlight) {
                case 0: start_burger_game(money, sock); break;
                case 1: start_package_game(money, sock); break;
                case 2: return -1; // 메인 메뉴로 돌아가기
            }
        }
    }
}
