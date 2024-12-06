#include "node.hpp"

Node::Node(string ip,uint16_t port){
    this->ip = ip;
    this->port = port;
    this->connection = new TCPSocket(ip,port);
}

Node::~Node(){
    delete connection;
}
