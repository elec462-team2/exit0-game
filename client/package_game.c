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

void start_package_game(int *money, int sock) {
    initscr();
    cbreak();
    noecho();
    curs_set(1);

    while (1) {
        int idx = rand() % REGION_COUNT;

        clear();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "=== 📦 코팡 물류 센터 🚚 ===");
        mvprintw(3, 2, "어서와요, 물류 분류 정확하게 부탁해요. 틀리면 일급 날아가요! ");
        show_region_mapping(5, 2);
        mvprintw(11, 2, "목적지: %s", regions[idx].region);
        mvprintw(13, 2, "물류 창고 이름을 적으세요 (A~E), * 그만 두려면 [q]를 누르세요...* : ");
        refresh();

        move(14, 2);
        refresh();

        int ch = getch();
        char upper_ch = toupper(ch);

        // Clear previous input display
        mvprintw(15, 2, "당신의 입력: ");
        mvprintw(15, 15, "%c", upper_ch);
        refresh();

        if (tolower(ch) == 'q') {
            mvprintw(17, 2, "그만 가게요? 또 봐요~ ");
            mvprintw(18, 2, "* 아무 키나 눌러서 나가세요... *");
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
            *money += 10;
            mvprintw(17, 2, "정답! 잘했어요. +10 points.");
        } else {
            *money -= 5;

            if (*money < 0) *money = 0;
            
            mvprintw(17, 2, "땡! 잘 좀 해봐요. -5 points.");
            mvprintw(18, 2, "이리로 보냈어야지: %c", regions[idx].center);
        }

        mvprintw(20, 2, "잔고: %d", *money);
        mvprintw(22, 2, "* 다음 목적지를 보려면 아무 키나 누르세요... *");
        refresh();
        getch();
    }

    endwin();
}