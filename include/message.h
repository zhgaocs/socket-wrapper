#ifndef MESSAGE_H
#define MESSAGE_H 1

#include <cstring>
#include <utility>

struct Header
{
    int type;
    int length;
    char src_ip[16];
    char dst_ip[16];
    unsigned short src_port;
    unsigned short dst_port;
};

struct Message
{
    Message() = default;
    ~Message();

    Header header;
    char *data = nullptr;
};

class MessageBuilder
{
public:
    MessageBuilder &setDestIP(const char *ip);
    MessageBuilder &setDestPort(int port);
    MessageBuilder &setSourceIP(const char *ip);
    MessageBuilder &setSourcePort(int port);
    MessageBuilder &setData(const char *data);
    Message build();

private:
    Message msg;
};

char *serialize(const Message &msg);
void deserialize(const char *buffer, Message &msg);

#endif