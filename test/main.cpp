#include <cstring>
#include <iostream>
#include <thread>
#include "tcpclient.h"
#include "tcpserver.h"

#define MAIN_FUNC_BUFSIZE 32

void client_foo(const TCPClient &client, uint16_t peer_port)
{
    char buf[MAIN_FUNC_BUFSIZE];
    char str[] = "Hello client";
    Message msg;

    for (;;)
    {
        int recv_len = client.receive(buf, MAIN_FUNC_BUFSIZE);

        if (recv_len)
        {
            deserialize(msg, buf, recv_len);
            std::cout << "Received from port#" << ntohs(msg.header.src_port) << msg.data << std::endl;
        }
        else
        {
            msg.setSrcPort(client.get_port())
                .setDstPort(peer_port)
                .setData(str, sizeof(str));

            int ret = serialize(buf, MAIN_FUNC_BUFSIZE, msg);
            client.send(buf, ret);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char *argv[])
{
    const char *serv_ip = "127.0.0.1";
    uint16_t serv_port = 12345;

    std::thread server_thread(
        [=]()
        {
            try
            {
                TCPServer server(serv_port);
                server.forward();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Server error: " << e.what() << std::endl;
            }
        });

    try
    {
        TCPClient client1, client2;

        client1.connect(serv_ip, serv_port);
        client2.connect(serv_ip, serv_port);

        uint16_t client1_port = client1.get_port();
        uint16_t client2_port = client2.get_port();

        std::thread client1_thread(client_foo(client1, client2_port));
        std::thread client2_thread(client_foo(client2, client1_port));

        client1_thread.join();
        client2_thread.join();
        server_thread.join();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}