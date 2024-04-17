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

Message &Message::setDestPort(uint16_t port)
{
    header.dst_port = htons(port);
    return *this;
}

Message &Message::setSourcePort(uint16_t port)
{
    header.src_port = htons(port);
    return *this;
}

Message &Message::setData(const char *data)
{
    if (this->data)
        delete[] this->data;

    size_t data_len = strlen(data);
    this->data = new char[data_len + 1];
    strncpy(this->data, data, data_len);
    this->data[data_len] = '\0';

    header.length = data_len + 1; // include '\0'

    return *this;
}

int serialize(const Message &msg, char *buf, size_t bufsize)
{
    if (bufsize < sizeof(Header) + msg.header.length)
        return -1;

    memcpy(buf, &msg.header, sizeof(Header));
    memcpy(buf + sizeof(Header), msg.data, msg.header.length);

    return sizeof(Header) + msg.header.length;
}

int deserialize(const char *buf, size_t bufsize, Message &msg)
{
    if (bufsize < sizeof(Header))
        return 0;

    Header header;
    memcpy(&header, buf, sizeof(Header));

    int rest_len = bufsize - sizeof(Header);
    if (rest_len < header.length)
        return 0;

    char *data = new (std::nothrow) char[header.length];
    if (!data)
        return -1;
    memcpy(data, buf + sizeof(Header), header.length);

    msg.header = std::move(header);
    if (!msg.data)
        delete[] msg.data;
    msg.data = data;

    return sizeof(Header) + msg.header.length;
}
