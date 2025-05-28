CC = gcc
CFLAGS = -Wall -Iinclude

CLIENT_SRCS = client/main.c client/login.c client/casino.c
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
CLIENT_EXEC = client_exec

SERVER_SRCS = server/main.c server/login.c server/session.c server/casino.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
SERVER_EXEC = server_exec

.PHONY: all clean

all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) -o $@ $^ -lncurses

$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_EXEC) $(SERVER_EXEC)