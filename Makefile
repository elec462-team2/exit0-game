CC      = gcc
CFLAGS  = -Wall -std=c99 -D_XOPEN_SOURCE=700 -Iinclude
LDLIBS  = -lncursesw

CLIENT_SRCS = client/main.c client/login.c client/casino.c client/ranking.c client/chat.c client/labor.c
SERVER_SRCS = server/main.c server/login.c server/session.c server/casino.c server/chat.c server/labor.c server/ranking.c

CLIENT_EXEC = client_exec
SERVER_EXEC = server_exec

CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
SERVER_OBJS = $(SERVER_SRCS:.c=.o)

all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRCS) $(LDLIBS)

$(SERVER_EXEC): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRCS) -lncursesw

clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_EXEC) $(SERVER_EXEC)