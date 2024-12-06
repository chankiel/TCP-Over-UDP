#ifndef SERVER_HPP
#define SERVER_HPP

#include "node.hpp"
#include "../Segment/segment.hpp"
#include <cstring>
#include <iostream>
#include <string>
#include "../Socket/socket.hpp"

class Server : public Node {
public:
  Server(int port): Node("127.0.0.1",8080){}
  void handleMessage(void *buffer,int sizeBuffer) override;
  void run();

private:
  int port_;
  std::string clientIp_ = "127.0.0.1"; // For simplicity, assuming a local client.
  int clientPort_ = 8081;              // Example client port.
  TCPSocket *connection;               // TCP connection object
};

#endif // SERVER_HPP
