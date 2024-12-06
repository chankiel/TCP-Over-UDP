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
  string item;
  uint16_t port;
  TCPSocket *connection;

public:
  Node(string ip, uint16_t port);
  ~Node();
  virtual void run() = 0;
  std::string getItem() const { return item; }
  void setItemFromBin(const std::string &binaryString);
  void setItem(const std::string &string);
};

#endif