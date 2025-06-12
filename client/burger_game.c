// burger_game.c
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../include/protocol.h"

// 🔥 함수 및 변수 선언
void update_user_asset(const char *userid, int new_balance);
extern char global_user_id[MAX_ID_LEN];

#define MAX_INGREDIENTS 6
#define MAX_ORDER 7

const char *ingredients[MAX_INGREDIENTS] = {
    "빵", "양상추", "토마토", "패티", "치즈", "양파"
};

void start_burger_game(int *money, int sock) {
    extern char global_user_id[MAX_ID_LEN];  // 유저 ID 전역변수 (client/main.c에 선언된 것)

    srand(time(NULL) ^ getpid());
    int running = 1;

    while (running) {
        clear(); box(stdscr, 0, 0);

        mvprintw(1, 4, " << 햄빌리버블 버거 가게 >>");
        mvprintw(3, 4, "버거 만들기 알바 하러 오셨죠? 앞치마 매고 이리오세요~");
        //mvprintw(2, 4, "Press 'q' to quit anytime.");

        // 1. Ingredient list
        mvprintw(5, 4, "재료 순서입니다. 바르게 넣지 않으면 손님이 화낼 지도 몰라요! :");
        for (int i = 0; i < MAX_INGREDIENTS; i++) {
            mvprintw(6 + i, 6, "[%d] %s", i + 1, ingredients[i]);
        }

        // 2. Order generation: first and last = Bun
        int order_len = 5 + rand() % 2;  // 5 or 6 ingredients
        int order[MAX_ORDER];
        order[0] = 0;  // Bun
        for (int i = 1; i < order_len - 1; i++) {
            order[i] = 1 + rand() % (MAX_INGREDIENTS - 1);
        }
        order[order_len - 1] = 0;  // Bun

        int x = 6;
        mvprintw(14, 4, "오늘의 주문:");
        for (int i = 0; i < order_len; i++) {
            const char *name = ingredients[order[i]];
            int len = strlen(name);
            mvprintw(15, x, "[%s]", name);
            x += len + 4;
        }

        mvprintw(17, 4, "순서에 맞게 재료를 입력하세요 (예시. 1 3 5 ...):");
        refresh();

        // 3. User input
        echo();
        char input_line[128] = {0};
        move(18, 4);
        getnstr(input_line, sizeof(input_line) - 1);
        noecho();

        if (input_line[0] == 'q') {
            running = 0;
            break;
        }

        // 4. Check answer
        int correct = 1;
        char *token = strtok(input_line, " ");
        for (int i = 0; i < order_len; i++) {
            if (!token) { correct = 0; break; }
            int input_num = atoi(token) - 1;
            if (input_num != order[i]) { correct = 0; }
            token = strtok(NULL, " ");
        }
        if (token != NULL) correct = 0;  // too many inputs

        // 5. Result
        clear(); box(stdscr, 0, 0);
        if (correct) {
            int reward = order_len * 10;
            *money += reward;
            mvprintw(3, 4, "\"이 버거, 제 인생을 바꿨어요.\"");
            mvprintw(6, 4, "✅ 완벽한 버거입니다. 손님도 만족하네요. ");
            mvprintw(8, 4, "[임금 지급] +%d G (총 자산: %d G)", reward, *money);
        } else {
            int penalty = 200;
            *money -= penalty;

            if (*money < 0) *money = 0;
            
            mvprintw(3, 4, "\"이런 버거 처음 봐! 사장 불러와!\"");
            mvprintw(6, 4, "❌ 버거가 잘못 나갔네요... 손님이 화났습니다.");
            mvprintw(8, 4, "[임금 차감] -%d G (총 자산: %d G)", penalty, *money);

            mvprintw(10, 4, "정답:");
            for (int i = 0; i < order_len; i++) {
                mvprintw(11, 6 + i*5, "%d", order[i]);
            }
        }

        mvprintw(15, 4, "* 메뉴로 나가려면 [Q], 계속 하려면 아무 키나 누르세요... *");
        refresh();
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            AssetUpdateRequest req = { .cmd = CMD_UPDATE_ASSET };
            strcpy(req.user_id, global_user_id);
            req.money = *money;
            send(sock, &req, sizeof(req), 0);
            break;
        }
    }

    clear(); box(stdscr, 0, 0);
    mvprintw(4, 4, "오늘 알바 수고했어요. 다음에도 시간 맞춰 와요!");
    refresh(); getch();

}