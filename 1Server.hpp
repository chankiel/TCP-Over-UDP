#ifndef server_h
#define server_h

#include "node.hpp"
#include <string>

/**
 * Abstract class.
 *
 * This is the base class for Server and Client class.
 */
class Server : public Node {
private:
  string server_ip;
  int server_port;

public:
  void handleMessage(void *buffer);
};

#endif