#include "../include/protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <ncurses.h>
#include <ctype.h>
#include "../include/server_api.h"

// ***package_game용 함수, 변수 선언
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

// 햄버거 게임 처리 
void handle_burger_game(int client_sock, const char *userid) {
    const char *ingredients[] = {"빵", "양상추", "토마토", "패티", "치즈", "양파"};
    int answer_indices[8];
    int order_len = 5 + rand() % 2;

    // 1. 요청 수신
    BurgerOrderRequest req;
    recv(client_sock, &req, sizeof(req), 0);

    // 2. 주문서 생성
    BurgerOrderSheet sheet = { .order_len = order_len };
    strcpy(sheet.order[0], "빵");
    answer_indices[0] = 0;
    for (int i = 1; i < order_len - 1; i++) {
        int idx = 1 + rand() % 5;
        strcpy(sheet.order[i], ingredients[idx]);
        answer_indices[i] = idx;
    }
    strcpy(sheet.order[order_len - 1], "빵");
    answer_indices[order_len - 1] = 0;

    send(client_sock, &sheet, sizeof(sheet), 0);

    // 3. 사용자 정답 수신
    CommandType cmd;
    recv(client_sock, &cmd, sizeof(cmd), 0);

    if (cmd != CMD_BURGER_RES) return;

    BurgerAnswerRequest answer;
    recv(client_sock, &answer, sizeof(answer), 0);

    // 👇 종료 신호 감지: "Q" 입력 시 바로 리턴
    if (strcmp(answer.input, "Q") == 0) {
        return;
    }

    // 4. 정답 비교
    int correct = 1;
    char *token = strtok(answer.input, " ");
    for (int i = 0; i < order_len; i++) {
        if (!token) { correct = 0; break; }
        int input_idx = atoi(token) - 1;
        if (input_idx != answer_indices[i]) correct = 0;
        token = strtok(NULL, " ");
    }
    if (token != NULL) correct = 0;  // 입력 초과

    // 5. 결과 전송
    int delta = correct ? 50 : -20;
    int money = get_user_asset(userid) + delta;
    update_user_asset(userid, money);

    BurgerResultResponse result = {
        .correct = correct,
        .delta_money = delta,
        .updated_money = money
    };
    send(client_sock, &result, sizeof(result), 0);
}

void handle_package_game(int client_sock, const char *userid) {
    // [1] 요청 수신
    PackageRequest req;
    recv(client_sock, &req, sizeof(req), 0);

    // [2] 지역 무작위 선택
    int idx = rand() % REGION_COUNT;
    RegionData selected = {
        .center = regions[idx].center
    };
    strncpy(selected.region, regions[idx].region, 10);
    selected.region[9] = '\0';  // 명시적으로 널 종료 추가  

    // [3] 지역 전송
    PackageResponse resp = { .region_info = selected };
    send(client_sock, &resp, sizeof(resp), 0);

    // [4] 사용자 응답 수신
    PackageAnswer answer;
    recv(client_sock, &answer, sizeof(answer), 0);

    // 👇 Q 누른 경우 처리: 바로 리턴
    if (toupper(answer.answer) == 'Q') {
        return;
    }

    // [5] 채점
    int correct = (toupper(answer.answer) == selected.center);
    int delta = correct ? 10 : -5;
    int money = get_user_asset(userid);
    money += delta;
    if (money < 0) money = 0;
    update_user_asset(userid, money);

    // [6] 결과 전송
    PackageResult result = {
        .correct = correct,
        .delta_money = delta,
        .updated_money = money,
        .correct_answer = selected.center
    };
    send(client_sock, &result, sizeof(result), 0);
}
