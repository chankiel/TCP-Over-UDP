#include "tools.hpp"

bool isNumber(const std::string &str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

void commandLine(char symbol, std::string str)
{
    std::cout << "[" << symbol << "] " << str << std::endl;
}

std::string brackets(std::string str)
{
    return " [" + str + "] ";
}

void commandLineHandshakeReceiver(
    int *ack,
    int *sync,
    std::string ip_handshake,
    int port_handshake)
{
    if (ack && sync)
    {
        commandLine('i', "[Handshake] [S=" + std::to_string(*sync) + "] [A=" + std::to_string(*ack) +
                             "] Received SYN-ACK request from " + ip_handshake + ":" + std::to_string(port_handshake));
    }
    else if (ack)
    {
        commandLine('i', "[Handshake] [A=" + std::to_string(*ack) +
                             "] Sending ACK request to " + ip_handshake + ":" + std::to_string(port_handshake));
    }
    else if (sync)
    {
        commandLine('i', "[Handshake] [S=" + std::to_string(*sync) +
                             "] Sending SYN request to " + ip_handshake + ":" + std::to_string(port_handshake));
    }
    else
    {
        std::cout << "Ben Ganteng\n";
    }
}

int generateRandomNumber(int min, int max)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine engine(seed);
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(engine);
}

std::string binaryToString(const std::string &binary)
{
    std::string originalString;

    if (binary.length() % 8 != 0)
    {
        std::cerr << "Invalid binary string length. It must be a multiple of 8." << std::endl;
        return "";
    }

    for (size_t i = 0; i < binary.length(); i += 8)
    {
        std::bitset<8> byte(binary.substr(i, 8));
        originalString += static_cast<char>(byte.to_ulong());
    }

    return originalString;
}

std::string stringToBinary(const std::string &input)
{
    std::string binaryString;

    for (char c : input)
    {
        binaryString += std::bitset<8>(c).to_string();
    }

    return binaryString;
}
