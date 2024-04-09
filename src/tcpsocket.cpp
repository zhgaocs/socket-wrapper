#include "tcpsocket.h"

TCPSocket::TCPSocket() : sockfd(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
{
}

TCPSocket::~TCPSocket()
{
    ::close(sockfd);
}