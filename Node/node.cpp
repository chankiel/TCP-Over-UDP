#include "node.hpp"

Node::Node(string ip, uint16_t port)
{
    this->ip = ip;
    this->port = port;
    this->connection = new TCPSocket(ip, port);
}

Node::~Node()
{
    delete connection;
}

void Node::setItem(const std::string &string)
{
    this->item = string;
}

void Node::setItemFromBin(const std::string &binaryString)
{
    if (binaryString.empty())
    {
        std::cerr << "Error: binary string is empty!" << std::endl;
        return;
    }

    std::cout << "Setting item with binary string of size: " << binaryString.size() << std::endl;

    item.clear();

    item.reserve(binaryString.size() / 8);

    for (size_t i = 0; i < binaryString.size(); i += 8)
    {
        if (i + 8 <= binaryString.size())
        {
            std::bitset<8> byte(binaryString.substr(i, 8));
            item.push_back(static_cast<char>(byte.to_ulong()));
        }
        else
        {
            std::cerr << "Error: Incomplete byte at the end of the binary string!" << std::endl;
        }
    }

    std::cout << "Item vector size after conversion: " << item.size() << std::endl;

    std::cout << "Binary data successfully set in the Node item." << std::endl;
}
