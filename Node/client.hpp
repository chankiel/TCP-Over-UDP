#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "node.hpp"
#include "../Segment/segment.hpp"
#include "../Socket/socket.hpp"
#include <string>
#include <iostream>
#include <thread>

class Client : public Node
{
public:
  Client(const std::string &myIP, int myport) : Node(myIP, myport) {}
  void run() override;
  void handleMessage(void *buffer, int sizeBuffer) override;

  ConnectionResult findBroadcast(string dest_ip, uint16_t dest_port);
  ConnectionResult startHandshake(string dest_ip, uint16_t );
  ConnectionResult startFin(string dest_ip, uint16_t dest_port, uint32_t seqNum); 

  void closeConnection();

private:
  std::string serverIp_;
  int serverPort_;
};

#endif // CLIENT_HPP
