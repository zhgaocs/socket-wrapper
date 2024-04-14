#include <iostream>
#include <thread>
#include "tcpclient.h"
#include "tcpserver.h"

void client_thread(const char *ip, int port, const char *message)
{
    char buf[RECV_BUF_SIZE];
    TCPClient client;
    client.connect(ip, port);

    try
    {
        std::cout << "Connected to server" << std::endl;

        while (true)
        {
            client.send(message);
            client.receive(buf, RECV_BUF_SIZE);
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
                server.run();
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
