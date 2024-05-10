#include "tcpsocket.h"

TcpSocket::TcpSocket()
    : sockfd(-1),
      connected(false)
{
}

TcpSocket::~TcpSocket()
{
    if (sockfd < 0)
        return;
    close(sockfd);
}

bool TcpSocket::is_connected() const
{
    return connected;
}

void TcpSocket::connect(const char *ip, uint16_t port)
{
    if (sockfd < 0 && (sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) < 0)
        throw std::runtime_error(strerror(errno));

    disconnect();

    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
        throw std::runtime_error(strerror(errno));

    if (::connect(sockfd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr)) < 0)
    {
        if (errno != EINPROGRESS)
            throw std::runtime_error(strerror(errno));
        else
        {
            pollfd pfd;
            pfd.fd = sockfd;
            pfd.events = POLLOUT;

            int poll_ret = poll(&pfd, 1, POLL_TIMEOUT_MS);

            if (poll_ret > 0)
            {
                int optval;
                socklen_t optlen = sizeof(optval);

                if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
                    throw std::runtime_error(strerror(errno));

                if (optval)
                    throw std::runtime_error(strerror(optval));

                connected = true;
            }
            else if (!poll_ret)
                connected = false;
            else
                throw std::runtime_error(strerror(errno));
        }
    }
    else
        connected = true;
}

void TcpSocket::disconnect()
{
    if (connected)
    {
        shutdown(sockfd, SHUT_RDWR);
        connected = false;
    }
}

ssize_t TcpSocket::read(char *buf, size_t bufsize) const
{
    if (!connected)
        return -1;

    size_t total_received = 0;
    ssize_t recv_len = 0;

    while (total_received < bufsize)
    {
        recv_len = ::read(sockfd, buf + total_received, bufsize - total_received);

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

ssize_t TcpSocket::write(const char *buf, size_t bufsize) const
{
    if (!connected)
        return -1;

    size_t total_sent = 0;
    ssize_t sent_len = 0;

    while (total_sent < bufsize)
    {
        sent_len = ::write(sockfd, buf + total_sent, bufsize - total_sent);

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

uint16_t TcpSocket::self_port() const
{
    if (!connected)
        return 0;
    else
    {
        sockaddr_in addr;
        socklen_t len = sizeof(addr);

        if (getsockname(sockfd, reinterpret_cast<sockaddr *>(&addr), &len) < 0)
            throw std::runtime_error(strerror(errno));

        return ntohs(addr.sin_port);
    }
}
