#ifndef SOCKET_H
#define SOCKET_H 1

#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>

class TCPSocket
{
public:
    TCPSocket();
    virtual ~TCPSocket();

public:
    ssize_t read(char *buf, size_t bufsize);
    ssize_t write(const char *msg);

protected:
    int sockfd;
};

#endif