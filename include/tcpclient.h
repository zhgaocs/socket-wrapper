#ifndef TCPCLIENT_H
#define TCPCLIENT_H 1

#include <arpa/inet.h>
#include "tcpsocket.h"

class TCPClient : public TCPSocket
{
public:
    TCPClient() = default;
    ~TCPClient() = default;

public:
    bool connect(const char *ip, int port);
};

#endif