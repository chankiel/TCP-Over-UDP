#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include "server.hpp"
#include "client.hpp"

bool isNumber(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

void commandLine(char symbol, std::string str) {
    std::cout<<"["<<symbol<<"] "<<str;
}

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
            std::cerr << "Invalid port provided. Using default port: 8080\n";
        }
    }

    // Display the result
    std::cout << "IP Address: " << ip << "\n";
    std::cout << "Port: " << port << "\n";

    commandLine('i', "Node started at " + ip +":"+std::to_string(port)+"\n");
    commandLine('?', "Please chose the operating mode\n");
    commandLine('?', "1. Sender\n");
    commandLine('?', "2. Receiver\n");
    commandLine('?', "Input: ");
    int operating_mode_choice;
    std::cin>>operating_mode_choice;
    if(operating_mode_choice == 1) {
        std::cout<<"Hehe";
        Server server(port);
        server.startServer();
    } else if(operating_mode_choice == 2) {
        std::cout<<"Hehe";
        Client client(ip, port);
        client.startHandshake();
    } else {
        throw std::runtime_error("Invalid argument");
    }


    return 0;
}
