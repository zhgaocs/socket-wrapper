#ifndef MACROS_H
#define MACROS_H 1

#define LISTEN_BACKLOG 128       /* used in ::listen() */
#define RECV_BUF_SIZE 128        /* used in ::recv() */
#define IDLE_MAX_WAIT_TIME 10000 /* millisecond, used in TCPServer::echo() */
#define EPOLL_WAIT_MAX_EVENTS 16 /* used in TCPServer::forward() */

#endif