#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include "Node/server.hpp"
#include "Node/client.hpp"

int main(int argc, char *argv[])
{
    // Default values
    std::string ip = "localhost";
    int port = 8080; // Default port

    // Process arguments
    if (argc > 1)
    { // Check if at least one argument is provided
        if (isNumber(argv[1]))
        { // If the first argument is a port
            port = std::stoi(argv[1]);
        }
        else
        { // Otherwise, it's an IP
            ip = argv[1];
        }
    }

    if (argc > 2)
    { // Check if the second argument (port) is provided
        if (isNumber(argv[2]))
        {
            port = std::stoi(argv[2]);
        }
        else
        {
            std::cerr << "Invalid port provided. Using default port: 8080\n";
        }
    }

    commandLine('i', "Node started at " + ip + ":" + std::to_string(port) + "\n");
    commandLine('?', "Please chose the operating mode\n");
    commandLine('?', "1. Sender (Server)\n");
    commandLine('?', "2. Receiver (Client)\n");
    commandLine('?', "Input: ");

    int operating_mode_choice;
    std::cin >> operating_mode_choice;

    if (operating_mode_choice == 1)
    {
        commandLine('+', "Node is now a sender\n");

        commandLine('i', "Sender Program’s Initialization\n");
        commandLine('?', "Please choose the sending mode\n");
        commandLine('?', "1. User input\n");
        commandLine('?', "2. File input\n");
        commandLine('?', "Input: ");

        int sending_mode_choice;
        std::cin >> sending_mode_choice;

        if (sending_mode_choice == 1)
        {
            commandLine('?', "Input mode chosen, please enter your input: ");
            std::string userInput;
            std::cin.ignore();
            std::getline(std::cin, userInput);
            commandLine('+', "User input has been successfully received.\n");
        }
        else if (sending_mode_choice == 2)
        {
            commandLine('?', "File mode chosen, please enter the file path: ");
            std::string filePath;
            std::cin >> filePath;
            commandLine('+', "File has been successfully read.\n");
        }
        else
        {
            throw std::runtime_error("Invalid sending mode choice");
        }

        commandLine('i', "Listening to the broadcast port for clients.\n");

        Server server(ip, port);
        server.run();
    }
    else if (operating_mode_choice == 2)
    {
        commandLine('+', "Node is now a receiver\n");
        commandLine('?', "Input the server program’s port: ");

        int serverPort;
        std::cin >> serverPort;

        commandLine('+', "Trying to contact the sender at " + ip + ":" + std::to_string(serverPort) + "\n");

        Client client(ip, serverPort);
        client.run();
    }
    else
    {
        throw std::runtime_error("Invalid argument");
    }

    return 0;
}
