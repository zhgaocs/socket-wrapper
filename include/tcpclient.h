#ifndef TCPCLIENT_H
#define TCPCLIENT_H 1

#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

class TCPClient
{
public:
    TCPClient();
    ~TCPClient();

public:
    TCPClient(const TCPClient &) = delete;
    TCPClient &operator=(const TCPClient &) = delete;

public:
    bool connect(const char *ip, uint16_t port) const;
    void close();
    ssize_t send(const void *msg, size_t length) const;
    ssize_t receive(void *buf, size_t bufsize) const;

protected:
    int sockfd;
};

#endif