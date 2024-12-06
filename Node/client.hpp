#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "node.hpp"
#include "../Segment/segment.hpp"
#include "../Socket/socket.hpp"
#include <string>
#include <iostream>
#include <thread>

class Client : public Node {
public:
  Client(const std::string &serverIp, int serverPort): Node(serverIp,serverPort){}
  void handleMessage(void *buffer, int sizeBuffer) override;
  void startClient();
  void startHandshake();

private:
  uint16_t servPort;
};

#endif // CLIENT_HPP
