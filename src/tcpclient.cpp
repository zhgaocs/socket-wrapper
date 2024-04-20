#include "tcpclient.h"

TCPClient::TCPClient()
    : sockfd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)),
      connected(false)
{
    if (sockfd < 0)
        throw std::runtime_error(strerror(errno));
}

TCPClient::~TCPClient()
{
    close();
}

bool TCPClient::connect(const char *ip, uint16_t port)
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
            pollfd pfd;
            pfd.fd = sockfd;
            pfd.events = POLLOUT;

            int poll_ret = poll(&pfd, 1, CONNECT_POLL_TIMEOUT_MS);

            if (poll_ret > 0)
            {
                int optval;
                socklen_t optlen = sizeof(optval);

                if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
                    throw std::runtime_error(strerror(errno));

                if (optval)
                    throw std::runtime_error(strerror(optval));

                connected = true;
                return true;
            }
            else if (!poll_ret)
            {
                connected = false;
                return false;
            }
            else
                throw std::runtime_error(strerror(errno));
        }
    }
    else
    {
        connected = true;
        return true;
    }
}

inline bool TCPClient::is_connected() const
{
    return connected;
}

void TCPClient::close()
{
    connected = false;
    if (sockfd < 0)
        return;
    ::close(sockfd);
    sockfd = -1;
}

ssize_t TCPClient::send(const char *buf, size_t bufsize) const
{
    if (!connected)
        return -1;

    size_t total_sent = 0;
    ssize_t sent_len = 0;

    pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLOUT;

    while (total_sent < bufsize)
    {
        sent_len = ::send(sockfd, buf + total_sent, bufsize - total_sent, 0);

        if (sent_len < 0)
        {
            if (EWOULDBLOCK == errno || EAGAIN == errno)
            {
                int poll_ret = poll(&pfd, 1, SEND_POLL_TIMEOUT_MS);

                if (poll_ret > 0)
                    continue;
                else if (!poll_ret)
                    break;
                else
                    throw std::runtime_error(strerror(errno));
            }
            else
                throw std::runtime_error(strerror(errno));
        }

        total_sent += sent_len;
    }

    return static_cast<ssize_t>(total_sent);
}

ssize_t TCPClient::receive(char *buf, size_t bufsize) const
{
    if (!connected)
        return -1;

    size_t total_received = 0;
    ssize_t recv_len = 0;

    pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLIN;

    while (total_received < bufsize)
    {
        recv_len = recv(sockfd, buf + total_received, bufsize - total_received, 0);

        if (recv_len < 0)
        {
            if (EWOULDBLOCK == errno || EAGAIN == errno)
            {
                int poll_ret = poll(&pfd, 1, RECV_POLL_TIMEOUT_MS);

                if (poll_ret > 0)
                    continue;
                else if (!poll_ret)
                    break;
                else
                    throw std::runtime_error(strerror(errno));
            }
            else
                throw std::runtime_error(strerror(errno));
        }
        else if (!recv_len)
            break;

        total_received += recv_len;
    }

    return static_cast<ssize_t>(total_received);
}
