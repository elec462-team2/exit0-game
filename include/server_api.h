#ifndef __SERVER_API_H__
#define __SERVER_API_H__

int handle_login(int client_sock, char *user_id_buf);
int check_user_credentials(const char *userid, const char *password);
int check_user_id_exists(const char *userid);
int get_user_asset(const char *userid);

#endif
