#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "node.hpp"
#include "segment.hpp"
#include "socket.hpp"
#include <string>
#include <iostream>

class Client : public Node {
public:
  Client(const std::string &serverIp, int serverPort);
  void handleMessage(void *buffer) override;
  void startHandshake();

private:
  std::string serverIp_;
  int serverPort_;
  TCPSocket *connection;
};

#endif // CLIENT_HPP
