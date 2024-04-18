#include "tcpserver.h"

TCPServer::TCPServer(uint16_t port)
    : listenfd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)),
      epollfd(epoll_create1(0))
{
    if (listenfd < 0)
    {
        if (epollfd < 0)
            ;
        else
            ::close(epollfd);
        throw std::runtime_error(strerror(errno));
    }

    if (epollfd < 0)
    {
        ::close(listenfd);
        throw std::runtime_error(strerror(errno));
    }

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

inline TCPServer::~TCPServer()
{
    close();
}

void TCPServer::close()
{
    for (auto &fd : connfds)
    {
        if (fd < 0)
            continue;
        ::close(fd);
        fd = -1;
    }

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

void TCPServer::aux_send(int fd, const char *msg, size_t length)
{
    ssize_t total_sent = 0;

    pollfd pfd;
    pfd.fd = sockfd;
    pfd.events = POLLOUT;

    while (total_sent < length)
    {
        ssize_t ret = send(fd, msg + total_sent, length - total_sent, 0);

        if (ret < 0)
        {
            if (EWOULDBLOCK == errno || EAGAIN == errno)
            {
                int poll_ret = poll(&pfd, 1, SEND_POLL_TIMEOUT_MS);

                if (poll_ret > 0)
                    continue;
                else
                    throw std::runtime_error(strerror(errno));
            }
            else
                throw std::runtime_error(strerror(errno));
        }

        total_sent += ret;
    }
}

void TCPServer::echo()
{
    int maxfd;
    fd_set readfds;
    char buf[RECV_BUF_SIZE];

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
            timeout.tv_sec = SERVER_MAX_IDLE_TIME_MS / 1000;
            timeout.tv_usec = SERVER_MAX_IDLE_TIME_MS % 1000;

            int activity = select(maxfd + 1, &readfds, nullptr, nullptr, &timeout);

            if (!activity)
            {
                close();
                break;
            }
            else if (activity < 0)
                throw std::runtime_error(strerror(errno));
        }
        else if (select(maxfd + 1, &readfds, nullptr, nullptr, nullptr) < 0)
            throw std::runtime_error(strerror(errno));

        if (FD_ISSET(listenfd, &readfds))
        {
            int new_connfd = accept(listenfd, nullptr, nullptr);
            if (new_connfd < 0)
                throw std::runtime_error(strerror(errno));

            int flags = fcntl(new_connfd, F_GETFL, 0);
            if (flags < 0)
                throw std::runtime_error(strerror(errno));

            if (fcntl(new_connfd, F_SETFL, flags | O_NONBLOCK) < 0)
                throw std::runtime_error(strerror(errno));

            connfds.emplace_back(new_connfd);
        }

        for (auto &fd : connfds)
        {
            if (FD_ISSET(fd, &readfds))
            {
                for (;;)
                {
                    ssize_t recv_len = recv(fd, buf, RECV_BUF_SIZE, 0);

                    if (recv_len < 0)
                        throw std::runtime_error(strerror(errno));
                    else if (!recv_len)
                    {
                        ::close(fd);
                        fd = -1;
                        break;
                    }
                    else
                        aux_send(fd, buf, recv_len);
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

    std::unordered_map<int, size_t> fd2idx; // the subscript of fd in vector
    std::unordered_map<uint16_t, int> port2fd;
    std::unordered_map<int, std::string> fd2msgdata;

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
                socklen_t addr_len = sizeof(clientaddr);

                int new_connfd = accept(listenfd, reinterpret_cast<sockaddr *>(&clientaddr), &addr_len);
                if (new_connfd < 0)
                    throw std::runtime_error(strerror(errno));

                int flags = fcntl(new_connfd, F_GETFL, 0);
                if (flags < 0)
                    throw std::runtime_error(strerror(errno));

                if (fcntl(new_connfd, F_SETFL, flags | O_NONBLOCK) < 0)
                    throw std::runtime_error(strerror(errno));

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = new_connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, new_connfd, &ev) < 0)
                    throw std::runtime_error(strerror(errno));

                fd2idx.emplace(new_connfd, connfds.size());
                port2fd.emplace(clientaddr.sin_port, new_connfd);
                connfds.emplace_back(new_connfd);
            }
            else
            {
                char buf[RECV_BUF_SIZE];
                int connfd = events[i].data.fd;

                ssize_t recv_len = recv(connfd, buf, RECV_BUF_SIZE, 0);

                if (recv_len < 0)
                    throw std::runtime_error(strerror(errno));
                else if (!recv_len)
                {
                    ::close(connfd);
                    connfds[fd2idx[connfd]] = -1;
                    continue;
                }
                else
                {
                    Message msg;
                    std::string &last_recv_data = fd2msgdata[connfd]; // remaining data from the last recv

                    if (last_recv_data.empty())
                    {
                        int ret = deserialize(buf, recv_len, msg);

                        if (ret < 0)
                            throw std::bad_alloc();
                        else if (!ret) // incomplete
                        {
                            last_recv_data.resize(recv_len);
                            memcpy(&last_recv_data[0], buf, recv_len);
                        }
                        else
                            aux_send(port2fd[msg.header.dst_port], msg.data, msg.header.length);
                    }
                    else
                    {
                        char larger_buf[RECV_BUF_SIZE << 1];
                        size_t bufsize = last_recv_data.size() + recv_len;
                        strncpy(larger_buf, last_recv_data.data(), last_recv_data.size());
                        strncpy(larger_buf + last_recv_data.size(), buf, recv_len);

                        for (;;)
                        {
                            int ret = deserialize(larger_buf, bufsize, msg);

                            if (ret < 0)
                                throw std::bad_alloc();
                            else if (!ret)
                            {
                                last_recv_data.resize(bufsize);
                                memcpy(&last_recv_data[0], larger_buf, bufsize);

                                break;
                            }
                            else
                            {
                                bufsize -= ret;
                                memmove(larger_buf, larger_buf + ret, bufsize);
                                aux_send(port2fd[msg.header.dst_port], msg.data, msg.header.length);
                            }
                        }
                    }
                }
            }
        }
    }
}
