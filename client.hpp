#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "node.hpp"
#include "segment.hpp"
#include "socket.hpp"
#include <iostream>
#include <string>

class Client : public Node {
public:
  Client(string myIP, int myport);
  void handleMessage(void *buffer) override;
  void startHandshake();

private:
  string clientIP;
  int clientPort;
  std::string serverIp_;
  int serverPort_;
  TCPSocket *connection;
};

#endif // CLIENT_HPP
