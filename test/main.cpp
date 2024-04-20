#include <cstring>
#include <iostream>
#include <thread>
#include "tcpclient.h"
#include "tcpserver.h"

#define BUF_SIZE 32

void client_thread(const char *ip, uint16_t port, const char *message)
{
    char buf[BUF_SIZE];
    try
    {
        TCPClient client;
        client.connect(ip, port);
        std::cout << "Connected to server" << std::endl;

        for (;;)
        {
            client.send(message, strlen(message));
            client.receive(buf, sizeof(buf));
            std::cout << buf << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

int main()
{
    const char *server_ip = "127.0.0.1";
    uint16_t server_port = 12345;

    std::thread server_thread(
        []()
        {
            try
            {
                TCPServer server(server_port);
                server.echo();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Server error: " << e.what() << std::endl;
            }
        });

    std::thread client1_thread(client_thread, server_ip, server_port, "client1: hello");
    std::thread client2_thread(client_thread, server_ip, server_port, "client2: hello");

    server_thread.join();
    client1_thread.join();
    client2_thread.join();
}