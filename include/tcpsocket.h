#ifndef SOCKET_H
#define SOCKET_H 1

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class TCPSocket
{
public:
    TCPSocket();
    virtual ~TCPSocket();

public:
    ssize_t receive(char *buffer, size_t size);
    bool send(const char *msg);

private:
    int sockfd;
};

#endif