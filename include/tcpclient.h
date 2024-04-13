#ifndef TCPCLIENT_H
#define TCPCLIENT_H 1

#include <arpa/inet.h>
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
    bool connect(const char *servip, int servport);
    void close();
    size_t send(const char* msg);
    size_t receive(char* buf, size_t bufsize);

protected:
    int sockfd;
};

#endif