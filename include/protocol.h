#include <stdint.h>
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define MAX_ID_LEN      20
#define MAX_PW_LEN      20
#define MAX_MSG_LEN     256

typedef uint32_t CommandType;

// 명령 코드 정의 (define으로 대체)
#define CMD_LOGIN_REQ     1
#define CMD_LOGIN_RES     2
#define CMD_HIGHLOW_REQ   3
#define CMD_HIGHLOW_RES   4
#define CMD_UPDATE_ASSET  5
#define CMD_ASSET_RES     6
#define CMD_LOGOUT_REQ    7
#define CMD_LOGOUT_RES    8


// 로그인 요청
typedef struct {
    CommandType cmd;
    char user_id[MAX_ID_LEN];
    char password[MAX_PW_LEN];
} LoginRequest;

// 로그인 응답
typedef struct {
    CommandType cmd;
    int success;
    int money;
    char message[MAX_MSG_LEN];
} LoginResponse;

// 하이앤로우 요청
typedef struct {
    CommandType cmd;        // 반드시 4바이트 고정형
    int  bet;        // 4바이트
    int  guess_num;  // 4바이트
} HighLowRequest;

// 하이앤로우 응답
typedef struct {
    CommandType cmd;
    int my_num;
    int cpu_num;
    int win;
    int new_money;
    int bet;
    int guess_num;
} HighLowResponse;

// 자산 요청
typedef struct {
    CommandType cmd;
    char user_id[MAX_ID_LEN];
    int money;
} AssetUpdateRequest;

// 자산 응답
typedef struct {
    CommandType cmd;
    char user_id[MAX_ID_LEN];
    int money;
} AssetResponse;

#endif // __PROTOCOL_H__