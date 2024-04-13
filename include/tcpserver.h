#ifndef TCPSERVER_H
#define TCPSERVER_H 1

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <vector>

#define RECV_BUF_SIZE 128

class TCPServer
{
public:
    TCPServer(int port);
    ~TCPServer();

public:
    TCPServer(const TCPServer &) = delete;
    TCPServer &operator=(const TCPServer &) = delete;

public:
    void run();
    void close();

protected:
    int listenfd;
    std::vector<int> connectfds;
};

#endif