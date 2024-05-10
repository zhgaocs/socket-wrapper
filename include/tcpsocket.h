#ifndef TCPSOCKET_H
#define TCPSOCKET_H 1

#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>

class TcpSocket
{
public:
    TcpSocket();
    ~TcpSocket();

public:
    TcpSocket(const TcpSocket &) = delete;
    TcpSocket &operator=(const TcpSocket &) = delete;

public:
    bool is_connected() const;
    void connect(const char *ip, uint16_t port);
    void disconnect();
    ssize_t read(char *buf, size_t bufsize) const;
    ssize_t write(const char *buf, size_t bufsize) const;

public:
    uint16_t self_port() const;

protected:
    int sockfd;
    bool connected;

protected:
    static constexpr int POLL_TIMEOUT_MS = 4000;
};

#endif