#include "tcpclient.h"

TCPClient::TCPClient() : sockfd(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
{
    if (-1 == sockfd)
        throw std::runtime_error(strerror(errno));
}

TCPClient::~TCPClient()
{
    close();
}

bool TCPClient::connect(const char *servip, int servport)
{
    if (sockfd != -1)
    {
        struct sockaddr_in serv_addr;
        std::memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(servport);

        if (inet_pton(AF_INET, servip, &serv_addr.sin_addr) <= 0)
            throw std::runtime_error(strerror(errno));

        if (::connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            throw std::runtime_error(strerror(errno));

        return true;
    }
    else
        throw std::runtime_error("Socket closed");
}

void TCPClient::close()
{
    if (sockfd != -1)
    {
        ::close(sockfd);
        sockfd = -1;
    }
}

size_t TCPClient::send(const char *msg)
{
    if (sockfd != -1)
    {
        ssize_t sent_len = ::send(sockfd, msg, strlen(msg), 0);

        if (-1 == sent_len)
            throw std::runtime_error(strerror(errno));
        return static_cast<size_t>(sent_len);
    }
    else
        throw std::runtime_error("Socket closed");
}

size_t TCPClient::receive(char *buf, size_t bufsize)
{
    if (sockfd != -1)
    {
        size_t total_received = 0;
        ssize_t recv_len = 0;

        while (total_received < bufsize)
        {
            int count;
            ioctl(sockfd, FIONREAD, &count);

            if (0 == count)
                break;

            recv_len = recv(sockfd, buf + total_received, bufsize - total_received, 0);

            if (0 == recv_len)
                break;

            else if (-1 == recv_len)
                throw std::runtime_error(strerror(errno));

            total_received += recv_len;
        }

        return total_received;
    }
    else
        throw std::runtime_error("Socket closed");
}