#include "message.hpp"
#include <iostream>

Message::Message(const std::string &ip, uint16_t port, const Segment &segment)
    : ip(ip), port(port), segment(segment) {}

Message::~Message() {}

Message::Message(const Message &other)
    : ip(other.ip), port(other.port), segment(other.segment) {}

Message::Message(Message &&other) noexcept
    : ip(std::move(other.ip)), port(other.port), segment(std::move(other.segment))
{
    other.port = 0;
}

Message &Message::operator=(const Message &other)
{
    if (this != &other)
    {
        ip = other.ip;
        port = other.port;
        segment = other.segment;
    }
    return *this;
}

Message &Message::operator=(Message &&other) noexcept
{
    if (this != &other)
    {
        ip = std::move(other.ip);
        port = other.port;
        segment = std::move(other.segment);
        other.port = 0;
    }
    return *this;
}

bool Message::operator==(const Message &other) const
{
    return (ip == other.ip) && (port == other.port) && (segment == other.segment);
}

std::ostream &operator<<(std::ostream &os, const Message &msg)
{
    os << "IP: " << msg.ip << ", Port: " << msg.port;
    printSegment(msg.segment);
    return os;
}

std::vector<Message> filterMessages(
    const std::vector<Message> &messages,
    const std::string &ip,
    uint16_t port,
    uint32_t seqNum)
{
    std::vector<Message> filteredMessages;

    for (const auto &msg : messages)
    {
        bool match = true;

        if (!ip.empty() && msg.ip != ip)
        {
            match = false;
        }

        if (port != 0 && msg.port != port)
        {
            match = false;
        }

        if (seqNum != 0 && msg.segment.seqNum != seqNum)
        {
            match = false;
        }

        if (match)
        {
            filteredMessages.push_back(msg);
        }
    }

    return filteredMessages;
}
