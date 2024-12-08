#include "Node/client.hpp"
#include "Node/server.hpp"
#include "Segment/segment.hpp"
#include "tools/fileReceiver.hpp"
#include "tools/fileSender.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

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

  Server server(ip, port);

  commandLine('i', "Node started at " + ip + ":" + std::to_string(port));
  commandLine('?', "Please chose the operating mode");
  commandLine('?', "1. Sender (Server)");
  commandLine('?', "2. Receiver (Client)");
  std::cout << INPUT << " Input: ";

  int operating_mode_choice;
  std::cin >> operating_mode_choice;

  if (operating_mode_choice == 1)
  {
    commandLine('+', "Node is now a sender");

    commandLine('i', "Sender Program's Initialization");
    commandLine('?', "Please choose the sending mode");
    commandLine('?', "1. User input");
    commandLine('?', "2. File input");
    std::cout << INPUT << " Input: ";

    int sending_mode_choice;
    std::cin >> sending_mode_choice;

    if (sending_mode_choice == 1)
    {
      commandLine('?', "Input mode chosen, please enter your input: ");
      std::string userInput;
      std::cin.ignore();
      std::getline(std::cin, userInput);
      server.setItem(userInput);
      server.setFileEx("-1");
      commandLine('+', "User input has been successfully received.");
    }
    else if (sending_mode_choice == 2)
    {
      cout<<INPUT<<" File mode chosen, please enter the file path: ";
      std::string filePath;
      std::cin.ignore();
      std::getline(std::cin, filePath);

      std::string transformedFilePath = transformFilePath(filePath);

      if (std::filesystem::exists(transformedFilePath))
      {
        commandLine('+', "File has been successfully read.");
        convertToFileContentAndSetItem(
            transformedFilePath, server);
        std::filesystem::path filePathObj(transformedFilePath);
        std::string fileName = filePathObj.stem().string();
        std::string fileExtension = filePathObj.extension().string();

        if (!fileExtension.empty() && fileExtension[0] == '.')
        {
          fileExtension.erase(0, 1);
        }
        server.setFileName(fileName);
        server.setFileEx(fileExtension);
      }
      else
      {
        commandLine('-', "Error: File does not exist at the specified path.");
        return 1;
      }
    }
    else
    {
      throw std::runtime_error("Invalid sending mode choice");
    }

    server.run();
  }
  else if (operating_mode_choice == 2)
  {
    commandLine('+', "Node is now a receiver");
  std::cout<<INPUT<<" Input the server program's port: ";

    int serverPort;
    std::cin >> serverPort;

    commandLine('+', "Trying to contact the sender at " + ip + ":" +
                         std::to_string(serverPort));

    Client client(ip, port, serverPort);
    client.run();
  }
  else
  {
    throw std::runtime_error("Invalid argument");
  }

  return 0;
}
