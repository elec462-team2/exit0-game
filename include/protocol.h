#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#define MAX_ID_LEN      20
#define MAX_PW_LEN      20
#define MAX_MSG_LEN     256
#define MAX_CARDS       10
#define MAX_TURNS       100

#define CMD_LOGIN_REQ     1
#define CMD_LOGIN_RES     2
#define CMD_HIGHLOW_REQ   3
#define CMD_HIGHLOW_RES   4
#define CMD_UPDATE_ASSET  5
#define CMD_ASSET_RES     6
#define CMD_LOGOUT_REQ    7
#define CMD_LOGOUT_RES    8
#define CMD_BLACKJACK_REQ     9
#define CMD_BLACKJACK_HIT    10
#define CMD_BLACKJACK_RESULT 11
#define CMD_BLACKJACK_RES    12
#define CMD_RACE_REQ  13
#define CMD_RACE_STEP 14
#define CMD_RACE_END 22

typedef uint32_t CommandType;

// 로그인 요청/응답
typedef struct {
    CommandType cmd;
    char user_id[MAX_ID_LEN];
    char password[MAX_PW_LEN];
} LoginRequest;

typedef struct {
    CommandType cmd;
    int success;
    int money;
    char message[MAX_MSG_LEN];
} LoginResponse;

// 하이앤로우
typedef struct {
    CommandType cmd;
    int  bet;
    int  guess_num;
} HighLowRequest;

typedef struct {
    CommandType cmd;
    int my_num;
    int cpu_num;
    int win;
    int new_money;
    int bet;
    int guess_num;
} HighLowResponse;

// 자산
typedef struct {
    CommandType cmd;
    char user_id[MAX_ID_LEN];
    int money;
} AssetUpdateRequest;

typedef struct {
    CommandType cmd;
    char user_id[MAX_ID_LEN];
    int money;
} AssetResponse;

// 블랙잭
typedef struct {
    CommandType cmd;
    int bet;
} BlackjackRequest;

typedef struct {
    CommandType cmd;
    int player_score;
    int dealer_score;
    int dealer_cards[MAX_CARDS]; // 딜러 카드 히스토리
    int dealer_card_count;       // 뽑은 카드 수
    int win;
    int bet;
    int new_money;
    int is_final;
} BlackjackResponse;

// 경마 게임 요청
typedef struct {
    CommandType cmd;
    int bet;
    int selected_horse; // 0, 1, 2 중 선택
} RaceRequest;

// 경마 게임 중 한 턴 위치 응답
typedef struct {
    CommandType cmd;  // CMD_RACE_STEP
    int horse_positions[3];  // 말 3마리의 현재 위치
    int finished;     // 0 = 진행 중, 1 = 끝남
} RaceStepResponse;

// 경마 게임 종료 후 최종 결과 응답
typedef struct {
    CommandType cmd;  // CMD_RACE_END
    int winner;       // 우승한 말 번호 (0~2)
    int user_choice;  // 사용자가 고른 말 번호
    int bet;
    int payout;
    int new_money;
} RaceResultResponse;

#endif // __PROTOCOL_H__