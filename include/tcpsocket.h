#ifndef SOCKET_H
#define SOCKET_H 1

#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>

class TCPSocket
{
protected:
    TCPSocket();
    virtual ~TCPSocket();

public:
    TCPSocket(const TCPSocket&) = delete;
    TCPSocket& operator=(const TCPSocket&) = delete;

public:
    size_t read(char *buf, size_t bufsize);
    size_t write(const char *msg);

protected:
    int sockfd;
};

#endif