#ifndef client_h
#define client_h

#include "node.hpp"

/**
 * Abstract class.
 *
 * This is the base class for Server and Client class.
 */
class Client : public Node {
public:
  void handleMessage(void *buffer);
};

#endif