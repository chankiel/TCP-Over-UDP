#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <cctype>
#include <algorithm>
#include "Node/server.hpp"
#include "Node/client.hpp"

std::string transformFilePath(const std::string &filePath)
{
    std::ostringstream transformedPath;
    for (char ch : filePath)
    {
        if (ch == '\\')
        {
            transformedPath << "\\\\";
        }
        else
        {
            transformedPath << ch;
        }
    }
    return transformedPath.str();
}

bool fileExists(const std::string &filePath)
{
    std::ifstream file(filePath);
    return file.good();
}

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

    commandLine('i', "Node started at " + ip + ":" + std::to_string(port));
    commandLine('?', "Please chose the operating mode");
    commandLine('?', "1. Sender (Server)");
    commandLine('?', "2. Receiver (Client)");
    commandLine('?', "Input: ");

    int operating_mode_choice;
    std::cin >> operating_mode_choice;

    if (operating_mode_choice == 1)
    {
        commandLine('+', "Node is now a sender");

        commandLine('i', "Sender Program’s Initialization");
        commandLine('?', "Please choose the sending mode");
        commandLine('?', "1. User input");
        commandLine('?', "2. File input");
        commandLine('?', "Input: ");

        int sending_mode_choice;
        std::cin >> sending_mode_choice;

        if (sending_mode_choice == 1)
        {
            commandLine('?', "Input mode chosen, please enter your input: ");
            std::string userInput;
            std::cin.ignore();
            std::getline(std::cin, userInput);
            commandLine('+', "User input has been successfully received.");
        }
        else if (sending_mode_choice == 2)
        {
            commandLine('?', "File mode chosen, please enter the file path: ");
            std::string filePath;
            std::cin.ignore();
            std::getline(std::cin, filePath);

            std::string transformedFilePath = transformFilePath(filePath);

            // std::cout << "Transformed file path: " << transformedFilePath << std::endl;

            // if (fileExists(filePath))
            // {
            //     commandLine('+', "File has been successfully read.\n");
            // }
            // else
            // {
            //     commandLine('-', "Error: File does not exist at the specified path.\n");
            //     return 1;
            // }

            // std::string filePath2 = "D:\\Personal Doc\\College\\Actual College\\Prak\\Prak 0 SBD\\P00_G00_13522043.docx";

            if (std::filesystem::exists(transformedFilePath))
            {
                commandLine('+', "File has been successfully read.");
            }
            else
            {
                commandLine('-', "Error: File does not exist at the specified path.");
                return 1;
            }

            // std::ifstream file(filePath);
            // if (file.is_open())
            // {
            //     commandLine('+', "File has been successfully read.\n");
            //     file.close();
            // }
            // else
            // {
            //     commandLine('-', "Error: File does not exist at the specified path.\n");
            //     return 1;
            // }
        }
        else
        {
            throw std::runtime_error("Invalid sending mode choice");
        }

        commandLine('i', "Listening to the broadcast port for clients.");

        Server server(ip, port);
        server.run();
    }
    else if (operating_mode_choice == 2)
    {
        commandLine('+', "Node is now a receiver");
        commandLine('?', "Input the server program's port: ");

        int serverPort;
        std::cin >> serverPort;

        commandLine('+', "Trying to contact the sender at " + ip + ":" + std::to_string(serverPort));

        Client client(ip, port,serverPort);
        client.run();
    }
    else
    {
        throw std::runtime_error("Invalid argument");
    }

    return 0;
}
