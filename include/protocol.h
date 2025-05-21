#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define MAX_ID_LEN      20
#define MAX_PW_LEN      20
#define MAX_MSG_LEN     256

// 명령 종류
typedef enum {
    CMD_LOGIN_REQ = 1,      // 로그인 요청
    CMD_LOGIN_RES,          // 로그인 응답
    CMD_CHAT_MSG,           // 채팅 메시지
    CMD_CHAT_BROADCAST,     // 채팅 브로드캐스트
    CMD_GAME_REQ,           // 게임 요청 (노동 or 도박)
    CMD_GAME_RES,           // 게임 결과 응답
    CMD_RANK_REQ,           // 랭킹 요청
    CMD_RANK_RES,           // 랭킹 응답
    CMD_LOGOUT_REQ,         // 로그아웃 요청
    CMD_LOGOUT_RES          // 로그아웃 완료
} CommandType;

// 로그인 요청 패킷
typedef struct {
    CommandType cmd;
    char user_id[MAX_ID_LEN];
    char password[MAX_PW_LEN];
} LoginRequest;

// 로그인 응답 패킷
typedef struct {
    CommandType cmd;
    int success;                    // 1=성공, 0=실패
    int money;                      // 자산 정보
    char message[MAX_MSG_LEN];     // 상태 메시지
} LoginResponse;

// 채팅 메시지
typedef struct {
    CommandType cmd;
    char sender[MAX_ID_LEN];
    char message[MAX_MSG_LEN];
} ChatMessage;

// 게임 결과 응답
typedef struct {
    CommandType cmd;
    int new_money;                 // 결과 반영 후 돈
    char result_msg[MAX_MSG_LEN]; // 예: "노동에 성공하여 100G 획득!"
} GameResponse;

// 랭킹 데이터 (복수 전송될 수 있음)
typedef struct {
    char user_id[MAX_ID_LEN];
    int money;
} RankEntry;

#endif // __PROTOCOL_H__
