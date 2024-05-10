#include <thread>

#include "tcpserver.h"
#include "tcpsocket.h"
#include "utils.h"

int sessions = 5;
int message_size = 16;

int main(int argc, char *argv[])
{
    parse_arguments(argv, argc);

    std::thread server_thread(
        []
        {
            try
            {
                TcpServer server;
                server.listen("0.0.0.0", 12345);
                server.run();
            }
            catch (const std::exception &e)
            {
                fprintf(stderr, "Server error: %s\n", e.what());
            }
        });
    server_thread.detach();

    // client thread
    {
        try
        {
            srand(time(nullptr));

            Message message;
            char *buf = new char[message_size];
            char *msg_buf = new char[message_size + sizeof(Header)];

            std::vector<TcpSocket> clients(sessions << 1);

            for (auto &client : clients)
                client.connect("127.0.0.1", 12345);

            for (;;)
            {
                for (int i = 0; i < clients.size(); ++i)
                {
                    if (i & 1) // receiver
                    {
                        if (clients[i].is_connected())
                        {
                            if (clients[i + 1].is_connected())
                            {
                                int real_bufsize = clients[i + 1].read(msg_buf, message_size + sizeof(Header));
                                deserialize(message, msg_buf, real_bufsize);

                                fprintf(stdout, "Client#%-5d(port#%-5d) recvs from port#%-5d: ",
                                        i, message.header.dst_port, message.header.src_port);
                                fwrite(message.data, sizeof(char), message.header.data_len, stdout);
                                fprintf(stdout, "\n");
                            }

                            if (!(rand() % 10))
                                clients[i].disconnect();
                        }
                        // else if (!(rand() % 10))
                        //     clients[i].connect("127.0.0.1", 12345);
                    }
                    else // sender
                    {
                        if (clients[i].is_connected())
                        {
                            if (clients[i + 1].is_connected())
                            {
                                uint16_t peer_port = clients[i + 1].self_port();
                                fill_buffer(buf, message_size);

                                message.setDstPort(peer_port)
                                    .setSrcPort(clients[i].self_port())
                                    .setData(buf, message_size);

                                serialize(msg_buf, message_size + sizeof(Header), message);
                                clients[i].write(msg_buf, message_size + sizeof(Header));


                                fprintf(stdout, "Client#%-5d(port#%-5d) sends to   port#%-5d: ",
                                        i, message.header.src_port, message.header.dst_port);
                                fwrite(message.data, sizeof(char), message.header.data_len, stdout);
                                fprintf(stdout, "\n");
                            }

                            if (!(rand() % 10))
                                clients[i].disconnect();
                        }
                        // else if (!(rand() % 10))
                        //     clients[i].connect("127.0.0.1", 12345);
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            fprintf(stderr, "Client error: %s\n", e.what());
        }
    }
}