#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

#define MAX_ID_LEN      20
#define MAX_PW_LEN      20
#define MAX_MSG_LEN     256

typedef uint32_t CommandType;

#define CMD_LOGIN_REQ     1
#define CMD_LOGIN_RES     2
#define CMD_REGISTER_REQ  3
#define CMD_REGISTER_RES  4
#define CMD_UPDATE_ASSET  5
#define CMD_ASSET_RES     6
#define CMD_LOGOUT_REQ    7
#define CMD_LOGOUT_RES    8

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

#endif
