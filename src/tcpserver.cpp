#include "tcpserver.h"

TCPServer::TCPServer(int port) : listenfd(socket(AF_INET, SOCK_STREAM, 0)), connectfds()
{
    if (-1 == listenfd)
        throw std::runtime_error(strerror(errno));

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (-1 == bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
    {
        ::close(listenfd);
        throw std::runtime_error(strerror(errno));
    }

    if (-1 == listen(listenfd, 10))
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
        if (fd != -1)
        {
            ::close(fd);
            fd = -1;
        }
    }
    if (listenfd != -1)
    {
        ::close(listenfd);
        listenfd = -1;
    }
}

void TCPServer::run()
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
            if (fd >= 0)
                FD_SET(fd, &readfds);

            if (fd > maxfd)
                maxfd = fd;
        }

        int activity = select(maxfd + 1, &readfds, nullptr, nullptr, nullptr);

        if ((activity < 0) && (errno != EINTR))
            throw std::runtime_error(strerror(errno));

        if (FD_ISSET(listenfd, &readfds))
        {
            int new_socket = accept(listenfd, nullptr, nullptr);

            if (new_socket < 0)
                throw std::runtime_error(strerror(errno));

            connectfds.emplace_back(new_socket);
        }

        for (auto &fd : connectfds)
        {
            if (FD_ISSET(fd, &readfds))
            {
                memset(buf, 0, sizeof(buf));
                int recv_len = recv(fd, buf, sizeof(buf), 0);

                if (-1 == recv_len)
                    throw std::runtime_error(strerror(errno));
                else if (0 == recv_len)
                {
                    ::close(fd);
                    fd = -1;
                }
                else
                {
                    while (recv_len > 0)
                    {
                        send(fd, buf, recv_len, 0);
                        memset(buf, 0, sizeof(buf));

                        int more_data;
                        ioctl(fd, FIONREAD, &more_data);

                        if (more_data > 0)
                            recv_len = recv(fd, buf, sizeof(buf), 0);
                        else
                            break;
                    }

                    if (-1 == recv_len)
                        throw std::runtime_error(strerror(errno));
                }
            }
        }
    }
}