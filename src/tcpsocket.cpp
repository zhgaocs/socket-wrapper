#include "tcpsocket.h"

TCPSocket::TCPSocket() : sockfd(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
{
    if (sockfd < 0)
        throw std::runtime_error("Failed to create socket");
}

TCPSocket::~TCPSocket()
{
    close(sockfd);
}

size_t TCPSocket::read(char *buf, size_t bufsize)
{
    ssize_t has_read = ::read(sockfd, buf, bufsize);
    if (has_read < 0)
        throw std::runtime_error("Read error");

    return has_read;
}

size_t TCPSocket::write(const char *msg)
{
    ssize_t has_written = ::write(sockfd, msg, strlen(msg));
    if (has_written < 0)
        throw std::runtime_error("Write error");

    return has_written;
}