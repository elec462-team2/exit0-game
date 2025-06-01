#ifndef __CLIENT_API_H__
#define __CLIENT_API_H__

// 로그인
int perform_login(int sock, char *user_id, int *user_money);
int login_loop(int sock, char *user_id, int *user_money);

// 게임
void enter_labor_game(int sock);
void enter_casino_game(int sock);
void start_casino_game(int sock, int *user_money);

// 채팅
void send_chat_message(int sock);
void receive_chat_broadcast(int sock);

#endif
