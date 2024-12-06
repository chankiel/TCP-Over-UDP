#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include "Node/server.hpp"
#include "Node/client.hpp"

int main(int argc, char* argv[]) {
    // Default values
    std::string ip = "localhost";
    int port = 8080; // Default port

    // Process arguments
    if (argc > 1) { // Check if at least one argument is provided
        if (isNumber(argv[1])) { // If the first argument is a port
            port = std::stoi(argv[1]);
        } else { // Otherwise, it's an IP
            ip = argv[1];
        }
    }

    if (argc > 2) { // Check if the second argument (port) is provided
        if (isNumber(argv[2])) {
            port = std::stoi(argv[2]);
        } else {
            std::cerr << "Invalid port provided. Using default port: 8080";
        }
    }

    commandLine('i', "Node started at " + ip +":"+std::to_string(port));
    commandLine('?', "Please chose the operating mode");
    commandLine('?', "1. Sender (Server)");
    commandLine('?', "2. Receiver (Client)");
    commandLine('?', "Input: ");
    int operating_mode_choice;
    std::cin>>operating_mode_choice;
    if(operating_mode_choice == 1) {
        commandLine('+', "Node is now a sender");
        Server server(ip,port);
        server.run();
    } else if(operating_mode_choice == 2) {
        commandLine('+', "Node is now a receiver");
        Client client(ip, port);
        client.run();
    } else {
        throw std::runtime_error("Invalid argument");
    }


    return 0;
}
