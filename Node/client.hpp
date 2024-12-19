#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "node.hpp"
#include <string>
#include <iostream>
#include <thread>

class Client : public Node
{
public:
  Client(const std::string &myIP, int myport, int serverPort) : Node(myIP, myport),serverPort(serverPort) {}
  void run() override; 
private:
  int serverPort;
};

#endif // CLIENT_HPP
