#include <chrono>
#include <iostream>
#include <thread>

#include "tcpclient.h"
#include "tcpserver.h"

#define MAIN_FUNC_BUFSIZE 32

int main(int argc, char *argv[])
{
    const uint16_t servport = 12345;

    std::thread server_thread(
        [servport]()
        {
            TCPServer server(servport);
            server.run();
        });

    server_thread.detach();
    
    std::cout.flush();
    try
    {
        TCPClient sender, receiver;
        sender.connect("127.0.0.1", servport);
        receiver.connect("127.0.0.1", servport);

        Message msg;
        char send_buf[] = "Hello receiver, No. ";
        char recv_buf[MAIN_FUNC_BUFSIZE];
        char msg_buf[sizeof(Header) + sizeof(send_buf)];

        for (int i = 0; i < 10;)
        {
            int recv_len = receiver.receive(recv_buf, sizeof(recv_buf));

            if(recv_len > 0)
            {
                if (deserialize(msg, recv_buf, recv_len))
                {
                    std::cout << "Receive from port#" << ntohs(msg.header.src_port) << ": ";
                    std::cout.write(msg.data, msg.header.data_len);
                    std::cout << std::endl;

                    ++i;
                }
            }

            send_buf[sizeof(send_buf) - 2] = 0x30 + i;
            msg.setSrcPort(sender.get_port())
                .setDstPort(receiver.get_port())
                .setData(send_buf, sizeof(send_buf));
            
            int ret = serialize(msg_buf, sizeof(msg_buf), msg);
            sender.send(msg_buf, ret);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client error: " << e.what() << std::endl;
    };
}