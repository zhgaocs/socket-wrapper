#ifndef TCPSERVER_H
#define TCPSERVER_H 1

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <unordered_map>
#include <vector>

#include "message.h"

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

public:
    TcpServer(const TcpServer &) = delete;
    TcpServer &operator=(const TcpServer &) = delete;

public:
    bool is_listening() const;
    void listen(const char *ip, uint16_t port);
    void close();
    void run();

protected:
    void aux_write(int fd, const char *buf, size_t bufsize);

protected:
    int listenfd;
    int epollfd;
    std::vector<int> connfds;

protected:
    static constexpr int LISTEN_BACKLOG = 32;
    static constexpr int RECV_BUFSIZE = 32;
    static constexpr int MAX_IDLE_TIME_MS = 10000;
    static constexpr int EPOLL_WAIT_MAX_EVENTS = 32;
};

#endif