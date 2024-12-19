#ifndef SERVER_HPP
#define SERVER_HPP

#include "../Segment/segment.hpp"
#include "../Socket/connection_result.hpp"
#include "../Socket/socket.hpp"
#include "node.hpp"
#include <cstring>
#include <iostream>
#include <string>
#include "../tools/tools.hpp"

class Server : public Node {
public:
  Server(string ip, int port): Node("0.0.0.0",port){}
  void run() override;
};

#endif // SERVER_HPP
