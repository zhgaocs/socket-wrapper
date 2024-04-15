#include <iostream>
#include <thread>
#include "tcpclient.h"
#include "tcpserver.h"

#ifndef BUF_SIZE
#define BUF_SIZE 32
#endif

void client_thread(const char *ip, int port, const char *message)
{
    try
    {
        char buf[BUF_SIZE];
        TCPClient client;
        client.connect(ip, port);
        std::cout << "Connected to server" << std::endl;

        for (;;)
        {
            client.send(message);
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
    int server_port = 12345;

    TCPServer server(server_port);

    std::thread server_thread(
        [&server]()
        {
            try
            {
                server.echoService();
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

    return 0;
}