#ifndef TCPCLIENT_H
#define TCPCLIENT_H 1

#include "tcpsocket.h"

class TCPClient : public TCPSocket
{
public:
    TCPClient();
    ~TCPClient();

public:
    bool connect(const char *ip, int port);
};

#endif