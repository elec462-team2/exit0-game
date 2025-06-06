// client/ranking.c
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/protocol.h"

void show_ranking() {
    clear();
    box(stdscr, 0, 0);
    mvprintw(1, 2, "=== Ranking ===");

    // 1. 기준 시각 출력
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);
    mvprintw(2, 2, "As of: %s", timebuf);

    // 2. asset_db.txt 읽기
    FILE *fp = fopen("data/asset_db.txt", "r");
    if (!fp) {
        mvprintw(4, 2, "Error: Cannot open asset_db.txt");
        refresh();
        getch();
        return;
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

    // 3. 정렬 (금액 내림차순)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (users[i].money < users[j].money) {
                User temp = users[i];
                users[i] = users[j];
                users[j] = temp;
            }
        }
    }

    // 4. 상위 30등 출력
    int limit = count < 30 ? count : 30;
    mvprintw(4, 2, "Rank  ID                Balance");
    mvprintw(5, 2, "---------------------------------");
    for (int i = 0; i < limit; i++) {
        mvprintw(6 + i, 2, "%2d. %-15s %8d G", i + 1, users[i].id, users[i].money);
    }

    mvprintw(17, 2, "Press any key to return...");
    refresh();
    getch();
}