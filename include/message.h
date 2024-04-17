#ifndef MESSAGE_H
#define MESSAGE_H 1

#include <netinet/in.h>

#include <cstring>
#include <stdexcept>
#include <utility>

struct Header
{
    uint32_t length;   /* contains '\0' in data*/
    uint16_t src_port; /* port in network byte order */
    uint16_t dst_port; /* port in network byte order */
};

struct Message
{
    Message();
    ~Message();
    Message(const Message &) = delete;
    Message &operator=(const Message &) = delete;

    Message &setDestPort(uint16_t port);
    Message &setSourcePort(uint16_t port);
    Message &setData(const char *data);

    Header header;
    char *data;
};

int serialize(const Message &msg, char *buf, size_t bufsize);
int deserialize(const char *buf, size_t bufsize, Message &msg);

#endif