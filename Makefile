# 컴파일러 및 옵션
CC = gcc
CFLAGS = -Wall -g
INCLUDES = -I./include

# 소스 파일 목록
CLIENT_SRCS = client/main.c client/login.c client/casino.c
SERVER_SRCS = server/main.c server/login.c server/casino.c server/session.c

# 오브젝트 파일 목록
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
SERVER_OBJS = $(SERVER_SRCS:.c=.o)

# 실행 파일명
CLIENT_EXEC = client_exec
SERVER_EXEC = server_exec

# 공통 규칙
.PHONY: all clean run debug

# 전체 빌드
all: $(CLIENT_EXEC) $(SERVER_EXEC)

# 클라이언트 빌드 (ncurses 포함)
$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

# 서버 빌드
$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 개별 .o 빌드
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 실행
run: all
	./$(SERVER_EXEC) 9000 & sleep 1; ./$(CLIENT_EXEC) 127.0.0.1 9000

# 디버그 실행
debug:
	gdb ./$(CLIENT_EXEC)

# 정리
clean:
	rm -f $(CLIENT_OBJS) $(SERVER_OBJS) $(CLIENT_EXEC) $(SERVER_EXEC)