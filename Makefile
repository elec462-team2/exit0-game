CC      = gcc
CFLAGS  = -Wall -Iinclude

CLIENT_SRCS = client/main.c client/login.c client/casino.c client/burger_game.c client/package_game.c client/ranking.c client/chat.c
SERVER_SRCS = server/main.c server/login.c server/session.c server/casino.c server/chat.c

CLIENT_EXEC = client_exec
SERVER_EXEC = server_exec

CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
SERVER_OBJS = $(SERVER_SRCS:.c=.o)


all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRCS) -lncursesw

$(SERVER_EXEC): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRCS)

clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_EXEC) $(SERVER_EXEC)