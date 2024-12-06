#ifndef SERVER_HPP
#define SERVER_HPP

#include "../Segment/segment.hpp"
#include "../Socket/connection_result.hpp"
#include "../Socket/socket.hpp"
#include "node.hpp"
#include <cstring>
#include <iostream>
#include <string>

class Server : public Node {
public:
  Server(string ip, int port) : Node(ip, port) {}
  void handleMessage(void *buffer, int sizeBuffer) override;
  void run();

  ConnectionResult listenBroadcast();
  ConnectionResult respondHandshake(string dest_ip, uint16_t);
  ConnectionResult respondFin(string dest_ip, uint16_t dest_port,
                              uint32_t seqNum);
};

#endif // SERVER_HPP
