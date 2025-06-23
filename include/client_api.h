//include/client_api.h
#ifndef __CLIENT_API_H__
#define __CLIENT_API_H__

// 로그인
int  perform_login(int sock, int *user_money);
int  perform_register(int sock);
void enter_chat_menu(int sock);

// 게임
void run_client(const char *ip, int port);
void start_casino_game(int sock, int *user_money);
void start_burger_game(int *money, int sock);
void start_package_game(int *money, int sock);

//tui 화면
int get_display_width(const char *str);
int get_centered_x(const char *text);
int get_centered_y(int lines);

#endif
