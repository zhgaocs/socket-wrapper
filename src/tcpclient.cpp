#include "tcpclient.h"

TCPClient::TCPClient()
    : sockfd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP))
{
    if (sockfd < 0)
        throw std::runtime_error(strerror(errno));
}

TCPClient::~TCPClient()
{
    close();
}

bool TCPClient::connect(const char *ip, int port) const
{
    if (sockfd < 0)
        return false;

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
        throw std::runtime_error(strerror(errno));

    if (::connect(sockfd, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0)
    {
        if (errno != EINPROGRESS)
            throw std::runtime_error(strerror(errno));
        else
        {
            struct pollfd pfd;
            pfd.fd = sockfd;
            pfd.events = POLLOUT;

            int poll_ret = poll(&pfd, 1, -1);

            if (poll_ret > 0)
            {
                int optval;
                socklen_t optlen = sizeof(optval);

                if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
                    throw std::runtime_error(strerror(errno));

                if (optval)
                    throw std::runtime_error(strerror(optval));

                return true;
            }
            else if (!poll_ret)
                return false;
            else
                throw std::runtime_error(strerror(errno));
        }
    }
    else
        return true;
}

void TCPClient::close()
{
    if (sockfd < 0)
        return;
    ::close(sockfd);
    sockfd = -1;
}

ssize_t TCPClient::send(const void *msg, size_t legnth) const
{
    if (sockfd < 0)
        return -1;

    size_t total_sent = 0;
    ssize_t sent_len = 0;

    while (total_sent < legnth)
    {
        sent_len = ::send(sockfd, msg + total_sent, legnth - total_sent, 0);

        if (sent_len < 0)
        {
            if (EWOULDBLOCK == errno || EAGAIN == errno)
                break;
            else
                throw std::runtime_error(strerror(errno));
        }

        total_sent += sent_len;
    }

    return static_cast<ssize_t>(total_sent);
}

ssize_t TCPClient::receive(void *buf, size_t bufsize) const
{
    if (sockfd < 0)
        return -1;
    size_t total_received = 0;
    ssize_t recv_len = 0;

    while (total_received < bufsize)
    {
        recv_len = recv(sockfd, buf + total_received, bufsize - total_received, 0);

        if (recv_len < 0)
        {
            if (EWOULDBLOCK == errno || EAGAIN == errno)
                break;
            else
                throw std::runtime_error(strerror(errno));
        }
        else if (!recv_len)
            break;

        total_received += recv_len;
    }

    return static_cast<ssize_t>(total_received);
}
