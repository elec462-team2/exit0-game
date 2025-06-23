// client/ranking.c
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include "../include/protocol.h"

void show_ranking(int sock) {
    clear(); box(stdscr, 0, 0);
    mvprintw(1, 2, "=== 📊 사용자 랭킹 ===");

    // 현재 시각 출력
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    mvprintw(2, 2, "현재 시각: %s", buf);

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
