#include "tcpclient.h"

bool TCPClient::connect(const char *ip, int port)
{
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0)
        return false;

    if (::connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        return false;

    return true;
}