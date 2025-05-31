#ifndef __CLIENT_API_H__
#define __CLIENT_API_H__

int  perform_login(int sock, int *user_money);
int  perform_register(int sock);
void run_client(const char *ip, int port);
void start_burger_game(int *money);
void start_package_game(int *money, int sock);


#endif
 