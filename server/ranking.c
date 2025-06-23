#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "../include/protocol.h"

void handle_ranking_request(int client_sock) {
    FILE *fp = fopen("data/asset_db.txt", "r");
    if (!fp) {
        perror("[SERVER] asset_db.txt 열기 실패");
        return;
    }

    RankingEntry users[1000];
    int count = 0;

    while (fscanf(fp, "%19[^:]:%d\n", users[count].id, &users[count].money) == 2) {
        count++;
    }
    fclose(fp);

    // 금액 기준 내림차순 정렬
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (users[i].money < users[j].money) {
                RankingEntry tmp = users[i];
                users[i] = users[j];
                users[j] = tmp;
            }
        }
    }

    RankingPacket res = { .count = count < 10 ? count : 10 };
    memcpy(res.entries, users, sizeof(RankingEntry) * res.count);

    send(client_sock, &res, sizeof(res), 0);
}
