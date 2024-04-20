#include "message.h"

Message::Message()
    : header(),
      data(nullptr)
{
}

Message::~Message()
{
    if (data)
        delete[] data;
}

Message &Message::setMsgType(MessageType type)
{
    header.msg_type = type;
    return *this;
}

Message &Message::setDstPort(uint16_t port)
{
    header.dst_port = htons(port);
    return *this;
}

Message &Message::setSrcPort(uint16_t port)
{
    header.src_port = htons(port);
    return *this;
}

Message &Message::setData(const char *buf, size_t bufsize)
{
    if (data)
    {
        delete[] data;
        header.data_len = 0;
    }

    if (!bufsize || !buf)
        return *this;

    data = new char[bufsize];
    memcpy(data, buf, /* sizeof(char) * */ bufsize);
    header.data_len = bufsize;

    return *this;
}

int serialize(char *buf, size_t bufsize, const Message &msg)
{
    if (bufsize < sizeof(Header) + msg.header.data_len)
        return 0;

    memcpy(buf, &msg.header, sizeof(Header));
    memcpy(buf + sizeof(Header), msg.data, /* sizeof(char) * */ msg.header.data_len);

    return sizeof(Header) + msg.header.data_len;
}

int deserialize(Message &msg, const char *buf, size_t bufsize)
{
    if (bufsize < sizeof(Header))
        return 0;

    Header header;
    memcpy(&header, buf, sizeof(Header));

    if (bufsize < sizeof(Header) + header.data_len)
        return 0;

    if (header.data_len)
    {
        char *data = new (std::nothrow) char[header.data_len];
        if (!data)
            return -1;
        memcpy(data, buf + sizeof(Header), /* sizeof(char) * */ header.data_len);

        if (msg.data)
            delete[] msg.data;
        msg.data = data;
    }
    else if (msg.data)
    {
        delete[] msg.data;
        msg.data = nullptr;
    }

    msg.header = std::move(header);

    return sizeof(Header) + msg.header.data_len;
}
