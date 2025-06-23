// client/ranking.c
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include "../include/protocol.h"
#include "../include/client_api.h"

void show_ranking(int sock) {
    clear(); box(stdscr, 0, 0);
        int y = 3;
    const char *title = "🏆 랭킹 보드 🏆";
    mvprintw(1, (COLS - get_display_width(title)) / 2, "%s", title);

    // 1. 기준 시각 출력
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    char time_line[128];
    snprintf(time_line, sizeof(time_line), "📅 현재 시각: %s", buf);
    mvprintw(y++, (COLS - get_display_width(time_line)) / 2, "%s", time_line);
    y++;

    // 랭킹 요청 전송
    CommandType cmd = CMD_RANKING_REQ;
    send(sock, &cmd, sizeof(cmd), 0);

    // 랭킹 수신
    RankingPacket packet;
    recv(sock, &packet, sizeof(packet), 0);

    mvprintw(4, 2, " 랭크     ID             자산");
    mvprintw(5, 2, "-------------------------------");
    for (int i = 0; i < packet.count; i++) {
        mvprintw(6 + i, 2, "%2d.     %-11s %8d G",
                 i + 1, packet.entries[i].id, packet.entries[i].money);
    }

    mvprintw(6 + packet.count + 2, 2, "* 돌아가려면 아무 키나 누르세요...");
    refresh(); getch();
}
