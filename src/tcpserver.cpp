#include "tcpserver.h"

TCPServer::TCPServer(int port)
    : listenfd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP))
{
    if (listenfd < 0)
        throw std::runtime_error(strerror(errno));

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, reinterpret_cast<struct sockaddr *>(&servaddr), sizeof(servaddr)) < 0)
    {
        ::close(listenfd);
        throw std::runtime_error(strerror(errno));
    }

    if (listen(listenfd, LISTEN_BACKLOG) < 0)
    {
        ::close(listenfd);
        throw std::runtime_error(strerror(errno));
    }
}

TCPServer::~TCPServer()
{
    close();
}

void TCPServer::close()
{
    for (auto &fd : connectfds)
    {
        if (fd < 0)
            continue;
        ::close(fd);
        fd = -1;
    }

    if (listenfd < 0)
        return;
    ::close(listenfd);
    listenfd = -1;
}

void TCPServer::echoService()
{
    fd_set readfds;
    int maxfd;
    char buf[RECV_BUF_SIZE];

    for (;;)
    {
        FD_ZERO(&readfds);
        FD_SET(listenfd, &readfds);
        maxfd = listenfd;

        for (auto &fd : connectfds)
        {
            if (fd < 0)
                continue;

            FD_SET(fd, &readfds);

            if (fd > maxfd)
                maxfd = fd;
        }

        if ((select(maxfd + 1, &readfds, nullptr, nullptr, nullptr) < 0) && (errno != EINTR))
            throw std::runtime_error(strerror(errno));

        if (FD_ISSET(listenfd, &readfds))
        {
            int new_socket = accept(listenfd, nullptr, nullptr);

            if (new_socket < 0)
                throw std::runtime_error(strerror(errno));

            int flags = fcntl(new_socket, F_GETFL, 0);
            fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);
            connectfds.emplace_back(new_socket);
        }

        for (auto &fd : connectfds)
        {
            if (FD_ISSET(fd, &readfds))
            {
                for (;;)
                {
                    ssize_t recv_len = recv(fd, buf, RECV_BUF_SIZE, 0);

                    if (recv_len < 0)
                    {
                        if (errno == EWOULDBLOCK || errno == EAGAIN)
                            break;
                        else
                            throw std::runtime_error(strerror(errno));
                    }
                    else if (recv_len == 0)
                    {
                        ::close(fd);
                        fd = -1;
                        break;
                    }
                    else
                    {
                        ssize_t total_sent = 0;
                        while (total_sent < recv_len)
                        {
                            ssize_t ret = send(fd, buf + total_sent, recv_len - total_sent, 0);
                            if (ret < 0)
                            {
                                if (errno == EWOULDBLOCK || errno == EAGAIN)
                                    continue; // To be optimized
                                else
                                    throw std::runtime_error(strerror(errno));
                            }
                            total_sent += ret;
                        }
                    }
                }
            }
        }
    }
}
