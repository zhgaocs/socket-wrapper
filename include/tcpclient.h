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
    bool connect(const char *servip, uint16_t servport);
    void close();
    ssize_t send(const char *buf, size_t bufsize) const;
    ssize_t receive(char *buf, size_t bufsize) const;

public:
    uint16_t get_port() const;

protected:
    int sockfd;
    bool connected;

protected:
    static constexpr int CONNECT_POLL_TIMEOUT_MS = 100;
    static constexpr int SEND_POLL_TIMEOUT_MS = 10;
    static constexpr int RECV_POLL_TIMEOUT_MS = 10;
};

#endif