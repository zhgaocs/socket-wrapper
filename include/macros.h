#ifndef MACROS_H
#define MACROS_H 1

/* used in TCPClient */
#define CONNECT_POLL_TIMEOUT_MS 10
#define SEND_POLL_TIMEOUT_MS 200
#define RECV_POLL_TIMEOUT_MS 200

/* used in TCPServer */
#define LISTEN_BACKLOG 64
#define RECV_BUF_SIZE 128
#define SERVER_MAX_IDLE_TIME_MS 10000
#define EPOLL_WAIT_MAX_EVENTS 16

#endif