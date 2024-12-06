#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#include <string>
#include "../Segment/segment.hpp"

class Message
{
public:
    string ip;
    uint16_t port;
    Segment segment;

    // Constructor
    Message(const std::string &ip, uint16_t port, const Segment &segment);

    // Destructor
    ~Message();

    // Copy constructor
    Message(const Message &other);

    // Move constructor
    Message(Message &&other) noexcept;

    // Assignment constructor
    Message &operator=(const Message &other);

    // Move assignment operator
    Message &operator=(Message &&other) noexcept;

    // Overload equality operator
    bool operator==(const Message &other) const;

    // Overload output stream operator
    friend std::ostream &operator<<(std::ostream &os, const Message &msg);
};

std::vector<Message> filterMessages(
    const std::vector<Message> &messages,
    const std::string &ip = "",
    uint16_t port = 0,
    uint32_t seqNum = 0);

#endif