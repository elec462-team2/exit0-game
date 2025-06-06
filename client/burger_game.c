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
    "Bun", "Lettuce", "Tomato", "Patty", "Cheese", "Onion"
};

void start_burger_game(int *money, int sock) {
    extern char global_user_id[MAX_ID_LEN];  // 유저 ID 전역변수 (client/main.c에 선언된 것)

    srand(time(NULL) ^ getpid());
    int running = 1;

    while (running) {
        clear(); box(stdscr, 0, 0);
        mvprintw(1, 4, "Burger Assembly Part-time Job!");
        mvprintw(2, 4, "Press 'q' to quit anytime.");

        // 1. Ingredient list
        mvprintw(4, 4, "Ingredient List:");
        for (int i = 0; i < MAX_INGREDIENTS; i++) {
            mvprintw(5 + i, 6, "[%d] %s", i + 1, ingredients[i]);
        }

        // 2. Order generation: first and last = Bun
        int order_len = 5 + rand() % 2;  // 5 or 6 ingredients
        int order[MAX_ORDER];
        order[0] = 0;  // Bun
        for (int i = 1; i < order_len - 1; i++) {
            order[i] = 1 + rand() % (MAX_INGREDIENTS - 1);
        }
        order[order_len - 1] = 0;  // Bun

        mvprintw(13, 4, "Today's Order:");
        for (int i = 0; i < order_len; i++) {
            mvprintw(14, 6 + i*10, "[%s]", ingredients[order[i]]);
        }

        mvprintw(16, 4, "Enter ingredient numbers in order (e.g., 1 3 5 ...):");
        refresh();

        // 3. User input
        echo();
        char input_line[128] = {0};
        move(17, 4);
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
            int reward = order_len * 100;
            *money += reward;
            mvprintw(4, 4, "✅ Perfect! Your burger was assembled correctly!");
            mvprintw(6, 4, "[Earnings] +%d G (Total: %d G)", reward, *money);
        } else {
            int penalty = 200;
            *money -= penalty;

            if (*money < 0) *money = 0;
            
            mvprintw(4, 4, "❌ Wrong ingredients! The customer is upset.");
            mvprintw(6, 4, "[Penalty] -%d G (Total: %d G)", penalty, *money);

            mvprintw(8, 4, "Correct Order:");
            for (int i = 0; i < order_len; i++) {
                mvprintw(9, 6 + i*10, "%s", ingredients[order[i]]);
            }
        }

        mvprintw(13, 4, "Press any key to continue (or 'q' to quit)");
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
    mvprintw(4, 4, "Exiting Burger Game. See you again!");
    refresh(); getch();

}