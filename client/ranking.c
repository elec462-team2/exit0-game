#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

void show_ranking() {
    clear();
    box(stdscr, 0, 0);

    int y = 3;
    const char *title = "🏆 랭킹 보드 🏆";
    mvprintw(1, (COLS - get_display_width(title)) / 2, "%s", title);

    // 현재 시각 출력
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);
    char time_line[128];
    snprintf(time_line, sizeof(time_line), "📅 현재 시각: %s", timebuf);
    mvprintw(y++, (COLS - get_display_width(time_line)) / 2, "%s", time_line);
    y++;

    FILE *fp = fopen("data/asset_db.txt", "r");
    if (!fp) {
        const char *msg = "asset 정보를 불러올 수 없습니다.";
        mvprintw(y, (COLS - get_display_width(msg)) / 2, "%s", msg);
        refresh(); getch(); return;
    }

    typedef struct {
        char id[MAX_ID_LEN];
        int money;
    } User;

    User users[1000];
    int count = 0;
    while (fscanf(fp, "%19[^:]:%d\n", users[count].id, &users[count].money) == 2) {
        count++;
    }
    fclose(fp);

    // 금액 내림차순 정렬
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (users[i].money < users[j].money) {
                User tmp = users[i]; users[i] = users[j]; users[j] = tmp;
            }
        }
    }

    int limit = count < 10 ? count : 10;
    const char *header = "순위     ID            자산 (G)";
    const char *divider = "---------------------------------";
    mvprintw(y++, (COLS - get_display_width(header)) / 2, "%s", header);
    mvprintw(y++, (COLS - get_display_width(divider)) / 2, "%s", divider);

    for (int i = 0; i < limit; i++) {
        char line[64];
        snprintf(line, sizeof(line), "%2d.   %-12s %10d", i + 1, users[i].id, users[i].money);
        mvprintw(y++, (COLS - get_display_width(line)) / 2, "%s", line);
    }

    const char *back = "🔙 아무 키나 누르면 돌아갑니다...";
    mvprintw(y + 2, (COLS - get_display_width(back)) / 2, "%s", back);
    refresh(); getch();
}