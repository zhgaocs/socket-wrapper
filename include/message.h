#ifndef MESSAGE_H
#define MESSAGE_H 1

#include <netinet/in.h>

#include <cstring>
#include <stdexcept>
#include <utility>

enum struct MessageType : uint16_t
{
    PORT_INFO,
    REG_MSG
};

struct Header
{
    MessageType msg_type;
    uint16_t data_len;
    uint16_t src_port; /* port in network byte order */
    uint16_t dst_port; /* port in network byte order */
};

struct Message
{
    Message();
    ~Message();
    Message(const Message &) = delete;
    Message &operator=(const Message &) = delete;

    Message &setMsgType(MessageType type);
    Message &setDstPort(uint16_t port);
    Message &setSrcPort(uint16_t port);
    Message &setData(const char *buf, size_t bufsize);

    Header header;
    char *data;
};

int serialize(char *buf, size_t bufsize, const Message &msg);
int deserialize(Message &msg, const char *buf, size_t bufsize);

#endif