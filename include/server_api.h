#ifndef __SERVER_API_H__
#define __SERVER_API_H__

#include "protocol.h"

// 로그인 관련
int check_user_credentials(const char *userid, const char *password);
void handle_login(int client_sock);

// 게임
void handle_labor_game(int client_sock, const char *userid);
void handle_casino_game(int client_sock, const char *userid);

// 채팅
void handle_chat(int client_sock, const char *userid);

#endif
