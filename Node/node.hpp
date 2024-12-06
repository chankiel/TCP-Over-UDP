#ifndef node_h
#define node_h

#include "../Socket/socket.hpp"

/**
 * Abstract class.
 *
 * This is the base class for Server and Client class.
 */
class Node
{
protected:
  string ip;
  uint16_t port;
  TCPSocket *connection;

public:
  Node(string ip, uint16_t port);
  ~Node();
  virtual void run() = 0;
};

#endif