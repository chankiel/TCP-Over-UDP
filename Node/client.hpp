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

  ConnectionResult findBroadcast(string dest_ip, uint16_t dest_port);
  ConnectionResult startHandshake(string dest_ip, uint16_t dest_port);
  ConnectionResult startFin(string dest_ip, uint16_t dest_port, uint32_t seqNum,uint32_t ackNum); 
};

#endif // CLIENT_HPP
