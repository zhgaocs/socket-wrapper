#ifndef TCPSERVER_H
#define TCPSERVER_H 1

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <vector>

#ifndef RECV_BUF_SIZE
#define RECV_BUF_SIZE 128
#endif

#ifndef LISTEN_BACKLOG
#define LISTEN_BACKLOG 128
#endif

class TCPServer
{
public:
    TCPServer(int port);
    ~TCPServer();

public:
    TCPServer(const TCPServer &) = delete;
    TCPServer &operator=(const TCPServer &) = delete;

public:
    void echoService();
    void close();

protected:
    int listenfd;
    std::vector<int> connectfds;
};

#endif