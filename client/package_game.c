#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include "../include/protocol.h"

#define REGION_COUNT 18

extern char global_user_id[MAX_ID_LEN];

typedef struct {
    char region[20];
    char center;
} RegionMap;

RegionMap regions[REGION_COUNT] = {
    {"Seoul", 'A'}, {"Incheon", 'A'}, {"Suwon", 'A'}, {"Goyang", 'A'},
    {"Busan", 'B'}, {"Daegu", 'B'}, {"Gyeongju", 'B'}, {"Pohang", 'B'},
    {"Gwangju", 'C'}, {"Jeonju", 'C'}, {"Mokpo", 'C'}, {"Gunsan", 'C'},
    {"Chuncheon", 'D'}, {"Wonju", 'D'}, {"Gangneung", 'D'}, {"Daejeon", 'D'},
    {"Jeju", 'E'}, {"Seogwipo", 'E'}
};

void show_region_mapping(int row, int col) {
    mvprintw(row, col,   "[A] Seoul, Incheon, Suwon, Goyang");
    mvprintw(row+1, col, "[B] Busan, Daegu, Gyeongju, Pohang");
    mvprintw(row+2, col, "[C] Gwangju, Jeonju, Mokpo, Gunsan");
    mvprintw(row+3, col, "[D] Chuncheon, Wonju, Gangneung, Daejeon");
    mvprintw(row+4, col, "[E] Jeju, Seogwipo");
}

void start_package_game(int *money, int sock) {
    initscr();
    cbreak();
    noecho();
    curs_set(1);

    while (1) {
        int idx = rand() % REGION_COUNT;

        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "=== Package Sorting Game ===");
        show_region_mapping(3, 2);
        mvprintw(9, 2, "Destination: %s", regions[idx].region);
        mvprintw(11, 2, "Type logistics center (A~E), or 'q' to quit:");
        refresh();

        move(12, 2);
        refresh();

        int ch = getch();
        char upper_ch = toupper(ch);

        // Clear previous input display
        mvprintw(14, 2, "Your input:   ");
        mvprintw(14, 13, "%c", upper_ch);
        refresh();

        if (tolower(ch) == 'q') {
            mvprintw(16, 2, "Quitting game... Press any key.");
            refresh();

            AssetUpdateRequest req = { .cmd = CMD_UPDATE_ASSET };
            strcpy(req.user_id, global_user_id);
            req.money = *money;
            send(sock, &req, sizeof(req), 0);

            refresh();
            getch();
            break;
        }

        if (upper_ch == regions[idx].center) {
            *money += 100;
            mvprintw(16, 2, "Correct! +100 points.");
        } else {
            *money -= 200;

            if (*money < 0) *money = 0;
            
            mvprintw(16, 2, "Wrong! -200 points.");
            mvprintw(17, 2, "The correct answer was: %c", regions[idx].center);
        }

        mvprintw(19, 2, "Current Balance: %d", *money);
        mvprintw(21, 2, "Press any key for next destination...");
        refresh();
        getch();
    }

    endwin();
}