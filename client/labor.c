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
    mvprintw(row,     get_centered_x("[A] 창고: 서울, 인천, 수원, 고양"), "[A] 창고: 서울, 인천, 수원, 고양");
    mvprintw(row + 1, get_centered_x("[B] 창고: 부산, 대구, 경주, 포항"), "[B] 창고: 부산, 대구, 경주, 포항");
    mvprintw(row + 2, get_centered_x("[C] 창고: 광주, 전주, 목포, 군산"), "[C] 창고: 광주, 전주, 목포, 군산");
    mvprintw(row + 3, get_centered_x("[D] 창고: 춘천, 원주, 강릉, 대전"), "[D] 창고: 춘천, 원주, 강릉, 대전");
    mvprintw(row + 4, get_centered_x("[E] 창고: 제주, 서귀포"), "[E] 창고: 제주, 서귀포");
}

void start_burger_game(int *money, int sock) {
    int running = 1;

    while (running) {
        clear();
        mvprintw(1, get_centered_x("<< 햄빌리버블 버거 가게 >>"), "<< 햄빌리버블 버거 가게 >>");
        mvprintw(3, get_centered_x("버거 만들기 알바 하러 오셨죠? 앞치마 매고 이리오세요~"),
                 "버거 만들기 알바 하러 오셨죠? 앞치마 매고 이리오세요~");

        CommandType cmd = CMD_BURGER_REQ;
        send(sock, &cmd, sizeof(cmd), 0);

        BurgerOrderRequest req = { .cmd = CMD_BURGER_REQ };
        strcpy(req.user_id, global_user_id);
        send(sock, &req, sizeof(req), 0);

        BurgerOrderSheet sheet;
        recv(sock, &sheet, sizeof(sheet), 0);

        mvprintw(5, get_centered_x("재료 순서입니다. 바르게 넣지 않으면 손님이 화낼 지도 몰라요! :"),
                 "재료 순서입니다. 바르게 넣지 않으면 손님이 화낼 지도 몰라요! :");
        for (int i = 0; i < 6; i++)
            mvprintw(6 + i, get_centered_x("[1] 빵") - 2, "[%d] %s", i + 1, ingredients[i]);

        mvprintw(14, get_centered_x("오늘의 주문:"), "오늘의 주문:");
        char order_line[128] = "";
        for (int i = 0; i < sheet.order_len; i++) {
            strcat(order_line, "[");
            strcat(order_line, sheet.order[i]);
            strcat(order_line, "] ");
        }
        mvprintw(15, get_centered_x(order_line), "%s", order_line);

        /*
        for (int i = 0; i < sheet.order_len; i++) {
            char temp[32];
            sprintf(temp, "[%s]", sheet.order[i]);
            mvprintw(15, get_centered_x(temp) + i * 8, "%s", temp);
        }
        */

        mvprintw(17, get_centered_x("순서에 맞게 재료를 입력하세요 (예시. 1 3 5 ...):"),
                 "순서에 맞게 재료를 입력하세요 (예시. 1 3 5 ...):");
        echo();
        char input_line[64] = {0};
        move(18, get_centered_x("입력칸"));
        getnstr(input_line, sizeof(input_line) - 1);
        noecho();

        if (tolower(input_line[0]) == 'q') {
            cmd = CMD_BURGER_RES;
            send(sock, &cmd, sizeof(cmd), 0);
            BurgerAnswerRequest quit_req = { .cmd = CMD_BURGER_RES };
            strcpy(quit_req.user_id, global_user_id);
            strcpy(quit_req.input, "Q");
            send(sock, &quit_req, sizeof(quit_req), 0);
            break;
        }

        cmd = CMD_BURGER_RES;
        send(sock, &cmd, sizeof(cmd), 0);

        BurgerAnswerRequest answer = { .cmd = CMD_BURGER_RES };
        strcpy(answer.user_id, global_user_id);
        strncpy(answer.input, input_line, sizeof(answer.input) - 1);
        send(sock, &answer, sizeof(answer), 0);

        BurgerResultResponse result;
        recv(sock, &result, sizeof(result), 0);

        clear(); box(stdscr, 0, 0);
        *money = result.updated_money;
        if (result.correct) {
            mvprintw(3, get_centered_x("\"이 버거, 제 인생을 바꿨어요.\""), "\"이 버거, 제 인생을 바꿨어요.\"");
            mvprintw(6, get_centered_x("✅ 완벽한 버거입니다. 손님도 만족하네요."), "✅ 완벽한 버거입니다. 손님도 만족하네요.");
            char reward_msg[64];
            sprintf(reward_msg, "[임금 지급] +%d G (총 자산: %d G)", result.delta_money, *money);
            mvprintw(8, get_centered_x(reward_msg), "%s", reward_msg);
        } else {
            mvprintw(3, get_centered_x("\"이런 버거 처음 봐! 사장 불러와!\""), "\"이런 버거 처음 봐! 사장 불러와!\"");
            mvprintw(6, get_centered_x("❌ 버거가 잘못 나갔네요... 손님이 화났습니다."),
                     "❌ 버거가 잘못 나갔네요... 손님이 화났습니다.");
            char reward_msg[64];
            sprintf(reward_msg, "[임금 차감] %d G (총 자산: %d G)", result.delta_money, *money);
            mvprintw(8, get_centered_x(reward_msg), "%s", reward_msg);
            //mvprintw(8, get_centered_x("[임금 차감]"), "[임금 차감] %d G (총 자산: %d G)", result.delta_money, *money);
        }

        mvprintw(15, get_centered_x("* 메뉴로 나가려면 [Q], 계속 하려면 아무 키나 누르세요... *"),
                 "* 메뉴로 나가려면 [Q], 계속 하려면 아무 키나 누르세요... *");
        refresh();

        char ch = getch();
        if (tolower(ch) == 'q') break;
    }

    clear(); box(stdscr, 0, 0);
    mvprintw(4, get_centered_x("오늘 알바 수고했어요. 다음에도 시간 맞춰 와요!"),
             "오늘 알바 수고했어요. 다음에도 시간 맞춰 와요!");
    mvprintw(8, get_centered_x("* 아무 키나 누르세요... *"), "* 아무 키나 누르세요... *");
    refresh(); getch();
}

void start_package_game(int *money, int sock) {
    initscr(); cbreak(); noecho(); curs_set(1);

    while (1) {
        PackageRequest req = { .cmd = CMD_PACKAGE_REQ };
        strcpy(req.user_id, global_user_id);
        send(sock, &req, sizeof(req), 0);

        PackageResponse resp;
        recv(sock, &resp, sizeof(resp), 0);

        clear(); box(stdscr, 0, 0);
        mvprintw(1, get_centered_x("=== \U0001f4e6 코팡 물류 센터 \U0001f69a ==="), "=== \U0001f4e6 코팡 물류 센터 \U0001f69a ===");
        mvprintw(3, get_centered_x("어서와요, 물류 분류 정확하게 부탁해요. 틀리면 일급 날아가요!"),
                 "어서와요, 물류 분류 정확하게 부탁해요. 틀리면 일급 날아가요!");
        show_region_mapping(5, 2);
        mvprintw(11, get_centered_x("목적지: " "서울"), "목적지: %s", resp.region_info.region);
        mvprintw(13, get_centered_x("물류 창고 이름을 적으세요 (A~E), * 그만 두려면 [q]를 누르세요...* : "),
                 "물류 창고 이름을 적으세요 (A~E), * 그만 두려면 [q]를 누르세요...* : ");

        move(14, get_centered_x("입력칸"));
        int ch = getch();
        char upper_ch = toupper(ch);

        if (tolower(ch) == 'q') {
            PackageAnswer quit = { .cmd = CMD_PACKAGE_RES, .answer = 'Q' };
            strcpy(quit.user_id, global_user_id);
            send(sock, &quit, sizeof(quit), 0);

            mvprintw(16, get_centered_x("그만 가게요? 또 봐요~ "), "그만 가게요? 또 봐요~ ");
            mvprintw(18, get_centered_x("* 아무 키나 눌러서 나가세요... *"), "* 아무 키나 눌러서 나가세요... *");
            refresh(); getch(); break;
        }

        PackageAnswer answer = { .cmd = CMD_PACKAGE_RES, .answer = upper_ch };
        strcpy(answer.user_id, global_user_id);
        send(sock, &answer, sizeof(answer), 0);

        PackageResult result;
        recv(sock, &result, sizeof(result), 0);

        mvprintw(16, get_centered_x("당신의 입력: X"), "당신의 입력: %c", toupper(ch));
        if (result.correct) {
            *money += result.delta_money;
            mvprintw(17, get_centered_x("정답! +10 G"), "정답! +10 G");
        } else {
            *money += result.delta_money;
            mvprintw(17, get_centered_x("오답! -5 G"), "오답! -5 G");
            mvprintw(18, get_centered_x("정답은: Z"), "정답은: %c", result.correct_answer);
        }

        mvprintw(20, get_centered_x("잔고: 0000 G"), "잔고: %d G", *money);
        mvprintw(22, get_centered_x("* 다음 문제는 아무 키나 누르세요... *"), "* 다음 문제는 아무 키나 누르세요... *");
        refresh(); getch();
    }

    curs_set(0); endwin();
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
                mvprintw(y + 2 + i, x - 2, "\u2794 %s", options[i]);
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
                case 2: return -1;
            }
        }
    }
}
