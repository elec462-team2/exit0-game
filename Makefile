CC      = gcc
CFLAGS  = -Wall -Iinclude

CLIENT_SRCS = client/main.c client/login.c client/burger_game.c client/package_game.c client/ranking.c
SERVER_SRCS = server/main.c server/login.c server/session.c

CLIENT_EXEC = client_exec
SERVER_EXEC = server_exec

CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
SERVER_OBJS = $(SERVER_SRCS:.c=.o)


all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRCS) -lncurses

$(SERVER_EXEC): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRCS)

clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_EXEC) $(SERVER_EXEC)
