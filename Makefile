CC = gcc
CFLAGS = -Wall -Iinclude

CLIENT_SRCS = client/main.c client/login.c
SERVER_SRCS = server/main.c server/login.c

CLIENT_EXEC = client_exec
SERVER_EXEC = server_exec

all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $(CLIENT_EXEC) $(CLIENT_SRCS)

$(SERVER_EXEC): $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $(SERVER_EXEC) $(SERVER_SRCS)

clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC)
