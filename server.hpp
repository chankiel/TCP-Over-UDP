#ifndef SERVER_HPP
#define SERVER_HPP

#include "node.hpp"
#include "segment.hpp"
#include "socket.hpp"
#include <cstring>
#include <iostream>
#include <string>

class Server : public Node {
public:
  Server(string ip, int port);
  void handleMessage(void *buffer) override;
  void startServer();

private:
  int serverPort;
  std::string serverIP;  // For simplicity, assuming a local client.
  TCPSocket *connection; // TCP connection object
};

#endif // SERVER_HPP
