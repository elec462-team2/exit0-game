#ifndef __CLIENT_API_H__
#define __CLIENT_API_H__

// 로그인
int  perform_login(int sock, int *user_money);
int  perform_register(int sock);

// 게임
void run_client(const char *ip, int port);
void start_casino_game(int sock, int *user_money);
void start_burger_game(int *money, int sock);
void start_package_game(int *money, int sock);

#endif
