#include <chrono>
#include <iostream>
#include <thread>

#include "tcpclient.h"
#include "tcpserver.h"

int main(int argc, char *argv[])
{
    const char *servip = "127.0.0.1";
    const uint16_t servport = 12345;

    std::thread server_thread(
        [servport]()
        {
            try
            {
                TCPServer server(servport);
                server.run();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Server error: " << e.what() << std::endl;
            };
        });

    server_thread.detach();
}