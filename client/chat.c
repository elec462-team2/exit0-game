// client/chat.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/socket.h>
#include "../include/protocol.h"

extern char global_user_id[MAX_ID_LEN];

// client/chat.c
void chat_view_inbox(int sock) {
    CommandType cmd = CMD_CHAT_INBOX_REQ;
    send(sock, &cmd, sizeof(cmd), 0);

    ChatInboxResponse res;
    memset(&res, 0, sizeof(res));  // 초기화

    // ✅ 순차적으로 recv
    recv(sock, &res.cmd, sizeof(CommandType), 0);
    recv(sock, &res.message_count, sizeof(int), 0);
    if (res.message_count > 0) {
        recv(sock, res.messages, sizeof(ChatMessage) * res.message_count, 0);
    }

    // ✅ 출력
    clear(); box(stdscr, 0, 0);
    mvprintw(1, 2, "📬  받은 메시지함 (%d개)", res.message_count);

    if (res.message_count == 0) {
        mvprintw(3, 4, "현재 받은 메시지가 없습니다.");
    } else {
        for (int i = 0; i < res.message_count; i++) {
            mvprintw(3 + i, 4, "[%s] %s", res.messages[i].from, res.messages[i].content);
        }
    }

    mvprintw(16, 2, "🔙  아무 키나 누르면 돌아갑니다...");
    refresh(); getch();
}

void chat_send_message(int sock) {
    char to[32], content[MAX_MSG_LEN];

    echo(); clear(); box(stdscr, 0, 0);
    mvprintw(2, 2, "✉️  메시지 보내기");
    mvprintw(4, 2, "받는 사람 ID: ");
    getnstr(to, sizeof(to) - 1);

    mvprintw(5, 2, "내용: ");
    getnstr(content, sizeof(content) - 1);
    noecho();

    CommandType cmd = CMD_CHAT_SEND_REQ;
    send(sock, &cmd, sizeof(cmd), 0);

    ChatSendRequest req;
    req.cmd = CMD_CHAT_SEND_REQ;
    strncpy(req.to, to, MAX_ID_LEN - 1);
    strncpy(req.content, content, MAX_MSG_LEN - 1);
    send(sock, &req, sizeof(req), 0);

    ChatSendResponse res;
    recv(sock, &res, sizeof(res), 0);

    mvprintw(7, 2, "결과: %s", res.message);
    mvprintw(9, 2, "🔙  아무 키나 누르면 돌아갑니다...");
    refresh(); getch();
}

void enter_chat_menu(int sock) {
    while (1) {
        clear(); box(stdscr, 0, 0);
        mvprintw(2, 2, "💬  채팅 메뉴");
        mvprintw(4, 4, "[1] 받은 메시지함");
        mvprintw(5, 4, "[2] 메시지 보내기");
        mvprintw(6, 4, "[Q] 뒤로 가기");
        mvprintw(8, 2, "선택: ");
        refresh();

        int ch = getch();
        if (ch == '1') chat_view_inbox(sock);
        else if (ch == '2') chat_send_message(sock);
        else if (ch == 'q' || ch == 'Q') break;
    }
}