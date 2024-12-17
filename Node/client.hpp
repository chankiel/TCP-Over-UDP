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
  Client(const std::string &myIP, int myport, int serverPort) : Node(myIP, myport),serverPort(serverPort) {}
  void run() override;

  ConnectionResult findBroadcast(string dest_ip, uint16_t dest_port);
  ConnectionResult startHandshake(string dest_ip, uint16_t dest_port);
  ConnectionResult respondFin(string dest_ip, uint16_t dest_port, uint32_t seqNum,uint32_t ackNum, uint32_t recfin_seqnum); 
private:
  int serverPort;
};

#endif // CLIENT_HPP
