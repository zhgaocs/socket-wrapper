#ifndef TCPSERVER_H
#define TCPSERVER_H 1

#include "tcpsocket.h"

class TCPServer : public TCPSocket
{
public:
    TCPServer();
    ~TCPServer();

public:
    bool bind(int port);
};

#endif