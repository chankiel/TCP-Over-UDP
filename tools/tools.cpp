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

std::string strip(const std::string& input, char character) {
    size_t start = input.find_first_not_of(character);
    if (start == std::string::npos) {
        return "";
    }

    size_t end = input.find_last_not_of(character);
    return input.substr(start, end - start + 1);
}