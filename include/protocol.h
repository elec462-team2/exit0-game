//include/protocol.h
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
#define CMD_REGISTER_REQ  3
#define CMD_REGISTER_RES  4
#define CMD_UPDATE_ASSET  5
#define CMD_ASSET_RES     6
#define CMD_LOGOUT_REQ    7
#define CMD_LOGOUT_RES    8
#define CMD_HIGHLOW_REQ   9
#define CMD_HIGHLOW_RES   10
#define CMD_BLACKJACK_REQ    11
#define CMD_BLACKJACK_HIT    12
#define CMD_BLACKJACK_RESULT 13
#define CMD_BLACKJACK_RES    14
#define CMD_RACE_REQ  15
#define CMD_RACE_STEP 16
#define CMD_RACE_END 17
#define CMD_CHAT_INBOX_REQ   18   // 채팅 메시지함 요청
#define CMD_CHAT_INBOX_RES   19   // 채팅 메시지함 응답
#define CMD_CHAT_SEND_REQ    20   // 메시지 보내기 요청
#define CMD_CHAT_SEND_RES    21   // 메시지 보내기 응답

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

typedef struct {
    CommandType cmd;
    char user_id[MAX_ID_LEN];
    char password[MAX_PW_LEN];
} RegisterRequest;

typedef struct {
    CommandType cmd;
    int success;
    char message[MAX_MSG_LEN];
} RegisterResponse;

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

// 블랙잭
typedef struct {
    CommandType cmd;
    int bet;
} BlackjackRequest;

typedef struct {
    CommandType cmd;
    int player_score;
    int dealer_score;
    int win;
    int bet;
    int new_money;
    int is_final;
    int dealer_card_count;
    int dealer_cards[MAX_CARDS];
    int last_card;
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

// 채팅 메시지 단위
typedef struct {
    char from[MAX_ID_LEN];             // 보낸 사람 ID
    char content[MAX_MSG_LEN];         // 메시지 내용
} ChatMessage;

// 메시지함 응답 구조
typedef struct {
    CommandType cmd;                   // CMD_CHAT_INBOX_RES
    ChatMessage messages[10];          // 최대 10개 메시지
    int message_count;                 // 실제 수신된 메시지 수
} ChatInboxResponse;

// 메시지 전송 요청
typedef struct {
    CommandType cmd;                   // CMD_CHAT_SEND_REQ
    char to[MAX_ID_LEN];               // 받는 사람 ID
    char content[MAX_MSG_LEN];         // 보낼 메시지
} ChatSendRequest;

// 메시지 전송 응답
typedef struct {
    CommandType cmd;                   // CMD_CHAT_SEND_RES
    int success;                       // 1=성공, 0=실패
    char message[MAX_MSG_LEN];         // 성공/실패 메시지
} ChatSendResponse;

#endif // __PROTOCOL_H__