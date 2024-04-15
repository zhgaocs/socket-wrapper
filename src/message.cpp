#include "message.h"

Message::~Message()
{
    if (data)
        delete[] data;
}

MessageBuilder &MessageBuilder::setDestIP(const char *ip)
{
    strncpy(msg.header.dst_ip, ip, sizeof(msg.header.dst_ip));
    msg.header.dst_ip[sizeof(msg.header.dst_ip) - 1] = '\0';
    return *this;
}

MessageBuilder &MessageBuilder::setDestPort(int port)
{
    msg.header.dst_port = port;
    return *this;
}

MessageBuilder &MessageBuilder::setSourceIP(const char *ip)
{
    strncpy(msg.header.src_ip, ip, sizeof(msg.header.src_ip));
    msg.header.src_ip[sizeof(msg.header.src_ip) - 1] = '\0';
    return *this;
}

MessageBuilder &MessageBuilder::setSourcePort(int port)
{
    msg.header.src_port = port;
    return *this;
}

MessageBuilder &MessageBuilder::setData(const char *data)
{
    if (msg.data)
        delete[] msg.data;
    msg.header.length = strlen(data);
    msg.data = new char[msg.header.length + 1];
    strncpy(msg.data, data, msg.header.length);
    msg.data[msg.header.length] = '\0';
    return *this;
}

Message MessageBuilder::build()
{
    Message result = std::move(msg);
    msg.data = nullptr;
    msg.header.length = 0;
    return result;
}

char *serialize(const Message &msg)
{
    char *buffer = new char[sizeof(msg.header) + msg.header.length];
    std::memcpy(buffer, &msg.header, sizeof(msg.header));
    std::memcpy(buffer + sizeof(msg.header), msg.data, msg.header.length);
    return buffer;
}

void deserialize(const char *buffer, Message &msg)
{
    std::memcpy(&msg.header, buffer, sizeof(msg.header));
    msg.data = new char[msg.header.length + 1];
    std::memcpy(msg.data, buffer + sizeof(msg.header), msg.header.length);
    msg.data[msg.header.length] = '\0';
}
