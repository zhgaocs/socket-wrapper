#include <thread>

#include "tcpserver.h"
#include "tcpsocket.h"
#include "utils.h"

int sessions = 100;
int message_size = 32;

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

            int connect_cnt = 0;

            char *buf = new char[message_size];
            char *msg_buf = new char[message_size + sizeof(Header)];

            Message message;
            std::vector<TcpSocket> clients(sessions << 1);

            for (auto &client : clients)
            {
                client.connect("127.0.0.1", 12345);
                if (client.is_connected())
                    ++connect_cnt;
            }

            while (connect_cnt)
            {
                for (int i = 0; i < clients.size(); ++i)
                {
                    if (i & 1) // receiver
                    {
                        if (clients[i].is_connected())
                        {
                            int real_bufsize = clients[i + 1].read(msg_buf, message_size + sizeof(Header));
                            deserialize(message, msg_buf, real_bufsize);

                            fprintf(stdout, "port#%-5d <- port#%-5d: ",
                                    message.header.dst_port, message.header.src_port);
                            fwrite(message.data, sizeof(char), message.header.data_len, stdout);
                            fprintf(stdout, "\n");

                            if (!(rand() % 10))
                            {
                                --connect_cnt;
                                fprintf(stdout, "port#%-5d disconnect\n", clients[i].self_port());
                                clients[i].disconnect();
                            }
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

                                fprintf(stdout, "port#%-5d -> port#%-5d: ",
                                        message.header.src_port, message.header.dst_port);
                                fwrite(message.data, sizeof(char), message.header.data_len, stdout);
                                fprintf(stdout, "\n");
                            }

                            if (!(rand() % 10) || !clients[i + 1].is_connected())
                            {
                                --connect_cnt;
                                fprintf(stdout, "port#%-5d disconnect\n", clients[i].self_port());
                                clients[i].disconnect();
                            }
                        }
                        // else if (!(rand() % 10))
                        //     clients[i].connect("127.0.0.1", 12345);
                    }
                }
            }

            fprintf(stdout, "client_thread ends\n");
            
            delete[] buf;
            delete[] msg_buf;
        }
        catch (const std::exception &e)
        {
            fprintf(stderr, "Client error: %s\n", e.what());
        }
    }
}