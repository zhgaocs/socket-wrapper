#ifndef TCPSERVER_H
#define TCPSERVER_H 1

#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include <cerrno>
#include <unordered_map>
#include <vector>

#include "message.h"

class TCPServer
{
public:
    TCPServer(uint16_t port);
    ~TCPServer();

public:
    TCPServer(const TCPServer &) = delete;
    TCPServer &operator=(const TCPServer &) = delete;

public:
    void echo();
    void forward();
    void close();

protected:
    static void aux_send(int fd, const char *buf, size_t length);

protected:
    int listenfd;
    int epollfd;
    std::vector<int> connfds;

protected:
    static constexpr int LISTEN_BACKLOG = 32;
    static constexpr int EPOLL_WAIT_MAX_EVENTS = 32;
    static constexpr int RECV_BUF_SIZE = 32;
    static constexpr int SEND_POLL_TIMEOUT_MS = 10;
    static constexpr int RECV_POLL_TIMEOUT_MS = 10;
    static constexpr int SERV_MAX_IDLE_TIME_MS = 1000;
};

#endif