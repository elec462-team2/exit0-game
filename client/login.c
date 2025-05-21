#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../include/protocol.h"

int perform_login(int sock) {
    LoginRequest req;
    LoginResponse res;

    req.cmd = CMD_LOGIN_REQ;

    printf("Enter ID: ");
    fgets(req.user_id, MAX_ID_LEN, stdin);
    req.user_id[strcspn(req.user_id, "\n")] = '\0';

    printf("Enter Password: ");
    fgets(req.password, MAX_PW_LEN, stdin);
    req.password[strcspn(req.password, "\n")] = '\0';

    send(sock, &req, sizeof(req), 0);

    ssize_t recv_len = recv(sock, &res, sizeof(res), 0);
    if (recv_len <= 0 || res.cmd != CMD_LOGIN_RES) {
        return -1;  // 서버 응답 이상
    }

    // 자산 출력
    if (res.success) {
        printf("✅ %s\n", res.message);
        printf("💰 보유 자산: %d G\n", res.money);
        return 1; // 성공
    } else {
        return 0; // 실패
    }
}