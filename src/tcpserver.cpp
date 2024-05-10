#include "tcpserver.h"

TcpServer::TcpServer()
    : listenfd(-1),
      epollfd(-1),
      connfds()
{
}

TcpServer::~TcpServer()
{
    for (auto fd : connfds)
        if (fd >= 0)
            ::close(fd);

    if (epollfd >= 0)
        ::close(epollfd);

    if (listenfd >= 0)
        ::close(listenfd);
}

bool TcpServer::is_listening() const
{
    return listenfd >= 0;
}

void TcpServer::listen(const char *ip, uint16_t port)
{
    close();

    listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd < 0)
        throw std::runtime_error(strerror(errno));

    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
        throw std::runtime_error(strerror(errno));

    if (bind(listenfd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr)) < 0)
        throw std::runtime_error(strerror(errno));

    if (::listen(listenfd, LISTEN_BACKLOG) < 0)
        throw std::runtime_error(strerror(errno));
}

void TcpServer::close()
{
    for (auto fd : connfds)
        if (fd >= 0)
            ::close(fd);

    connfds.clear();

    if (epollfd >= 0)
    {
        ::close(epollfd);
        epollfd = -1;
    }

    if (listenfd >= 0)
    {
        ::close(listenfd);
        listenfd = -1;
    }
}

void TcpServer::run()
{
    epollfd = epoll_create1(0);
    if (epollfd < 0)
        throw std::runtime_error(strerror(errno));

    size_t connect_cnt = 0;

    epoll_event ev, evs[EPOLL_WAIT_MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0)
        throw std::runtime_error(strerror(errno));

    std::unordered_map<int, size_t> fd2idx; // the subscript of connfd in vector
    std::unordered_map<uint16_t, int> port2fd;
    std::unordered_map<int, std::string> fd2msgdata;

    for (;;)
    {
        int nfds;

        if (!connect_cnt)
            nfds = epoll_wait(epollfd, evs, EPOLL_WAIT_MAX_EVENTS, MAX_IDLE_TIME_MS);
        else
            nfds = epoll_wait(epollfd, evs, EPOLL_WAIT_MAX_EVENTS, -1);

        if (nfds < 0)
        {
            if (EINTR == errno)
                continue;
            else
                throw std::runtime_error(strerror(errno));
        }

        if (!nfds && !connect_cnt)
            break;

        for (int i = 0; i < nfds; ++i)
        {
            if (evs[i].data.fd == listenfd)
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

                ++connect_cnt;
                fd2idx.emplace(new_connfd, connfds.size());
                port2fd.emplace(clientaddr.sin_port, new_connfd);
                connfds.emplace_back(new_connfd);
            }
            else
            {
                char buf[RECV_BUFSIZE];
                int connfd = evs[i].data.fd;

                ssize_t recv_len = read(connfd, buf, RECV_BUFSIZE);

                if (recv_len < 0)
                    throw std::runtime_error(strerror(errno));
                else if (!recv_len)
                {
                    --connect_cnt;
                    ::close(connfd);
                    connfds[fd2idx[connfd]] = -1;
                    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, nullptr) < 0)
                        throw std::runtime_error(strerror(errno));
                    // continue;
                }
                else
                {
                    Message msg;
                    std::string &last_recv_data = fd2msgdata[connfd]; // remaining data from the last recv

                    if (last_recv_data.empty())
                    {
                        for (;;)
                        {
                            int ret = deserialize(msg, buf, recv_len);

                            if (ret < 0)
                                throw std::bad_alloc();
                            else if (!ret) // incomplete
                            {
                                last_recv_data.resize(recv_len);
                                memcpy(&last_recv_data[0], buf, /* sizeof(char) * */ recv_len);
                                break;
                            }
                            else
                            {
                                aux_write(port2fd[msg.header.dst_port], buf, ret);
                                recv_len -= ret;
                                memmove(buf, buf + ret, recv_len);
                            }
                        }
                    }
                    else
                    {
                        char larger_buf[RECV_BUFSIZE << 1];
                        size_t len = last_recv_data.size() + recv_len;
                        memcpy(larger_buf, last_recv_data.data(), /* sizeof(char) * */ last_recv_data.size());
                        memcpy(larger_buf + last_recv_data.size(), buf, /* sizeof(char) * */ recv_len);

                        for (;;)
                        {
                            int ret = deserialize(msg, larger_buf, len);

                            if (ret < 0)
                                throw std::bad_alloc();
                            else if (!ret) // incomplete
                            {
                                last_recv_data.resize(len);
                                memcpy(&last_recv_data[0], larger_buf, /* sizeof(char) * */ len);
                                break;
                            }
                            else
                            {
                                aux_write(port2fd[msg.header.dst_port], larger_buf, ret);
                                len -= ret;
                                memmove(larger_buf, larger_buf + ret, len);
                            }
                        }
                    }
                }
            }
        }
    }
}

void TcpServer::aux_write(int fd, const char *buf, size_t bufsize)
{
    ssize_t total_sent = 0;

    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLOUT;

    while (total_sent < bufsize)
    {
        ssize_t ret = write(fd, buf + total_sent, bufsize - total_sent);

        if (ret < 0)
        {
            if (EWOULDBLOCK == errno || EAGAIN == errno)
            {
                int poll_ret = poll(&pfd, 1, -1);

                if (poll_ret > 0)
                    continue;
                else
                    throw std::runtime_error(strerror(errno));
                continue;
            }
            else
                throw std::runtime_error(strerror(errno));
        }

        total_sent += ret;
    }
}