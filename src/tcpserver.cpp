#include "tcpserver.h"

TCPServer::TCPServer(uint16_t port)
    : listenfd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)),
      epollfd(epoll_create1(0))
{
    if (listenfd < 0 || epollfd < 0)
        throw std::runtime_error(strerror(errno));

    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr)) < 0)
    {
        ::close(listenfd);
        ::close(epollfd);
        throw std::runtime_error(strerror(errno));
    }

    if (listen(listenfd, LISTEN_BACKLOG) < 0)
    {
        ::close(listenfd);
        ::close(epollfd);
        throw std::runtime_error(strerror(errno));
    }
}

TCPServer::~TCPServer()
{
    close();
}

void TCPServer::close()
{
    if (epollfd < 0)
        ;
    else
    {
        ::close(epollfd);
        epollfd = -1;
    }

    if (listenfd < 0)
        ;
    else
    {
        ::close(listenfd);
        listenfd = -1;
    }
}

void TCPServer::echo()
{
    int maxfd;
    fd_set readfds;
    char buf[RECV_BUF_SIZE];

    std::vector<int> connfds;
    auto close_connectfd = [&connfds]()
    {
        for (auto fd : connfds)
        {
            if (fd >= 0)
                ::close(fd);
        }
    };

    for (;;)
    {
        FD_ZERO(&readfds);
        FD_SET(listenfd, &readfds);
        maxfd = listenfd;

        bool idle = true;

        for (auto &fd : connfds)
        {
            if (fd < 0)
                continue;

            idle = false;
            FD_SET(fd, &readfds);

            if (fd > maxfd)
                maxfd = fd;
        }

        if (idle)
        {
            timeval timeout;
            timeout.tv_sec = IDLE_MAX_WAIT_TIME / 1000;
            timeout.tv_usec = IDLE_MAX_WAIT_TIME % 1000;

            int activity = select(maxfd + 1, &readfds, nullptr, nullptr, &timeout);

            if (!activity)
                break;
            else if (activity < 0 && (errno != EINTR))
                throw std::runtime_error(strerror(errno));
        }
        else if ((select(maxfd + 1, &readfds, nullptr, nullptr, nullptr) < 0) && (errno != EINTR))
        {
            close_connectfd();
            throw std::runtime_error(strerror(errno));
        }

        if (FD_ISSET(listenfd, &readfds))
        {
            int new_socket = accept(listenfd, nullptr, nullptr);
            if (new_socket < 0)
            {
                close_connectfd();
                throw std::runtime_error(strerror(errno));
            }

            int flags = fcntl(new_socket, F_GETFL, 0);
            if (flags < 0)
            {
                close_connectfd();
                throw std::runtime_error(strerror(errno));
            }

            if (fcntl(new_socket, F_SETFL, flags | O_NONBLOCK) < 0)
            {
                close_connectfd();
                throw std::runtime_error(strerror(errno));
            }
            connfds.emplace_back(new_socket);
        }

        for (auto &fd : connfds)
        {
            if (FD_ISSET(fd, &readfds))
            {
                for (;;)
                {
                    ssize_t recv_len = recv(fd, buf, RECV_BUF_SIZE, 0);

                    if (recv_len < 0)
                    {
                        if (EWOULDBLOCK == errno || EAGAIN == errno)
                            break;
                        else
                        {
                            close_connectfd();
                            throw std::runtime_error(strerror(errno));
                        }
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
                                if (EWOULDBLOCK == errno || EAGAIN == errno)
                                    continue; // To be optimized
                                else
                                {
                                    close_connectfd();
                                    throw std::runtime_error(strerror(errno));
                                }
                            }
                            total_sent += ret;
                        }
                    }
                }
            }
        }
    }
}

void TCPServer::forward()
{
    epoll_event ev, events[EPOLL_WAIT_MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0)
        throw std::runtime_error(strerror(errno));

    std::unordered_map<uint16_t, int> port2fd;
    std::unordered_map<int, std::string> message_buffers;

    for (;;)
    {
        int nfds = epoll_wait(epollfd, events, EPOLL_WAIT_MAX_EVENTS, -1);

        if (nfds < 0)
        {
            if (EINTR == errno)
                continue;
            else
                throw std::runtime_error(strerror(errno));
        }

        for (int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == listenfd)
            {
                sockaddr_in clientaddr;

                int new_socket = accept(listenfd, reinterpret_cast<sockaddr *>(&clientaddr), &sizeof(clientaddr));
                if (new_socket < 0)
                    throw std::runtime_error(strerror(errno));

                int flags = fcntl(new_socket, F_GETFL, 0);
                if (flags < 0)
                    throw std::runtime_error(strerror(errno));

                if (fcntl(new_socket, F_SETFL, flags | O_NONBLOCK) < 0)
                    throw std::runtime_error(strerror(errno));

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = new_socket;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, new_socket, &ev) < 0)
                    throw std::runtime_error(strerror(errno));

                port2fd.emplace(clientaddr.sin_port, new_socket);
            }
            else
            {
                char buf[RECV_BUF_SIZE];
                int connfd = events[n].data.fd;

                ssize_t recv_len = recv(connfd, buf, RECV_BUF_SIZE, 0);

                if (recv_len < 0)
                {
                    if (EWOULDBLOCK == errno || EAGAIN == errno)
                        continue;
                    else
                        throw std::runtime_error(strerror(errno));
                }
                else if (!recv_len)
                {
                    ::close(connfd);
                    buffers.erase(connfd);
                    continue;
                }
                else
                {
                    std::string &message_buffer = message_buffers[connfd];
                    
                    if (message_buffer.empty())
                    {
                        int ret = deserialize(buf, recv_len, msg);

                        if (ret < 0)
                            throw std::bad_alloc();
                        else if (!ret)
                        {
                            message_buffer.resize(recv_len);

                            for (int j = 0; j < recv_len; ++j)
                                message_buffer[j] = buf[j];
                        }
                        else
                        {
                            // TODO
                        }
                    }
                    else
                    {
                        char larger_buf[RECV_BUF_SIZE << 1];
                        size_t bufsize = message_buffer.size() + recv_len;
                        strncpy(larger_buf, message_buffer.data(), message_buffer.size());
                        strncpy(larger_buf + message_buffer.size(), buf, recv_len);

                        for (;;)
                        {
                            int ret = deserialize(larger_buf, bufsize, msg);

                            if (ret < 0)
                                throw std::bad_alloc();
                            else if (!ret)
                            {
                                message_buffer.resize(bufsize);

                                for (int j = 0; j < bufsize; ++j)
                                    message_buffer[j] = larger_buf[j];

                                break;
                            }
                            else
                            {
                                bufsize -= ret;
                                memove(larger_buf, larger_buf + ret, bufsize);

                                // TODO
                            }
                        }
                    }
                }
            }
        }
    }
}
