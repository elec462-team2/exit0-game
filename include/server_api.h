#ifndef __SERVER_API_H__
#define __SERVER_API_H__

// 로그인
int handle_login(int client_sock, char *user_id_buf);
int check_user_credentials(const char *userid, const char *password);


// 자산 조회·업데이트
int  get_user_asset(const char *userid);
void update_user_asset(const char *userid, int new_balance);

// 게임
void handle_labor_game(int client_sock, const char *userid);
void handle_casino_game(int client_sock, const char *userid);

// 채팅
void handle_chat(int client_sock, const char *userid);

#endif // __SERVER_API_H__