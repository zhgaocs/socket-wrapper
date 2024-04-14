#ifndef TCPCLIENT_H
#define TCPCLIENT_H 1

#include <arpa/inet.h>
#include <sys/ioctl.h>
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
    TCPClient(const TCPClient&) = delete;
    TCPClient& operator=(const TCPClient&) = delete;

public:
    bool connect(const char *ip, int port);
    void close();
    ssize_t send(const char* msg);
    ssize_t receive(char* buf, size_t bufsize);

protected:
    int sockfd;
};

#endif