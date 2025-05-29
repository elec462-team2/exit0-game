#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#define MAX_ID_LEN      20
#define MAX_PW_LEN      20
#define MAX_MSG_LEN     256

typedef enum {
    CMD_LOGIN_REQ = 1,
    CMD_LOGIN_RES,
    CMD_REGISTER_REQ,
    CMD_REGISTER_RES
} CommandType;

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

#endif
