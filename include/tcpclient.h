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
    bool is_connected() const;
    bool connect(const char *ip, uint16_t port);
    void close();
    ssize_t send(const char *msg, size_t length) const;
    ssize_t receive(char *buf, size_t bufsize) const;

protected:
    int sockfd;
    bool connected;
};

#endif