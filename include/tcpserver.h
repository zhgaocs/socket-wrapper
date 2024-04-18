#ifndef TCPSERVER_H
#define TCPSERVER_H 1

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "macros.h"
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
    static void aux_send(int fd, const char* buf, size_t length);

protected:
    int listenfd;
    int epollfd;
    std::vector<int> connfds;
};

#endif