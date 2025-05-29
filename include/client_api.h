#ifndef __CLIENT_API_H__
#define __CLIENT_API_H__

int  perform_login(int sock);
int  perform_register(int sock);
void run_client(const char *ip, int port);

#endif
