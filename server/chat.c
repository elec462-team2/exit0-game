// server/chat.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "../include/protocol.h"
#include "../include/server_api.h"

#define CHAT_DIR "data/inbox"

void handle_chat(int client_sock, const char *userid, CommandType cmd) {
    if (cmd == CMD_CHAT_SEND_REQ) {
        ChatSendRequest req;
        recv(client_sock, &req, sizeof(req), 0);

        // 받은 사람의 inbox 파일 경로 구성
        char path[128];
        snprintf(path, sizeof(path), "%s/%s.txt", CHAT_DIR, req.to);

        FILE *fp = fopen(path, "a");
        ChatSendResponse res = { .cmd = CMD_CHAT_SEND_RES };

        if (!fp) {
            res.success = 0;
            strcpy(res.message, "⚠️  메세지 전송 실패: 상대방을 찾을 수 없습니다. ");
        } else {
            fprintf(fp, "%s:%s\n", userid, req.content);
            fclose(fp);
            res.success = 1;
            strcpy(res.message, "✅  메세지가 성공적으로 전송되었습니다! ");
        }
        send(client_sock, &res, sizeof(res), 0);
    }

    else if (cmd == CMD_CHAT_INBOX_REQ) {
        ChatInboxResponse res = {
            .cmd = CMD_CHAT_INBOX_RES,
            .message_count = 0
        };
        
        // 내 메시지함 경로
        char path[128];
        snprintf(path, sizeof(path), "%s/%s.txt", CHAT_DIR, userid);
        FILE *fp = fopen(path, "r");
        if (fp) {
            char line[MAX_MSG_LEN + MAX_ID_LEN + 2];
            while (fgets(line, sizeof(line), fp) && res.message_count < 10) {
                char *sep = strchr(line, ':');
                if (!sep) continue;
                *sep = '\0';
                strcpy(res.messages[res.message_count].from, line);
                strncpy(res.messages[res.message_count].content, sep + 1, MAX_MSG_LEN);

                // 줄 끝 개행 제거
                size_t len = strlen(res.messages[res.message_count].content);
                if (len > 0 && res.messages[res.message_count].content[len - 1] == '\n')
                    res.messages[res.message_count].content[len - 1] = '\0';

                res.message_count++;
            }
            fclose(fp);
            remove(path);  // 읽은 후 삭제
        }

        send(client_sock, &res.cmd, sizeof(CommandType), 0);
        send(client_sock, &res.message_count, sizeof(int), 0);
        if (res.message_count > 0) {
            send(client_sock, res.messages, sizeof(ChatMessage) * res.message_count, 0);
        }
    }
}