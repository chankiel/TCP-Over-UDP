#include "node.hpp"
#include <cstring>
#include <iostream>

class Client : public Node {
public:
  Client(const std::string &serverIp, int serverPort) {
    connection = new TCPSocket(8080);
    serverIp_ = serverIp;
    serverPort_ = serverPort;
  }

  void handleMessage(void *buffer) override {
    Segment *segment = static_cast<Segment *>(buffer);
    if (segment->flags.syn == 1 && segment->flags.ack == 1) {
      std::cout << "Received SYN-ACK. Sending ACK..." << std::endl;
      Segment ackSegment = ack(0, 1);
      connection->send(serverIp_, serverPort_, &ackSegment, sizeof(ackSegment),
                       1);
      std::cout << "Handshake completed!" << std::endl;
    }
  }

  void startHandshake() {
    // connection->listen();
    Segment synSegment = syn(0);
    connection->send(serverIp_, serverPort_, &synSegment, sizeof(synSegment),
                     1);
    std::cout << "Sent SYN packet to server." << std::endl;
    void *buffer = malloc(sizeof(Segment));
    std::cout << "sebelum recv starthandshake" << std::endl;
    connection->ambil(buffer, sizeof(Segment), 1);
    std::cout << "sesudah recv starthandshake" << std::endl;
    handleMessage(buffer);
    free(buffer);
  }

private:
  std::string serverIp_;
  int serverPort_;
};

int main() {
  Client client("127.0.0.1", 8080);
  client.startHandshake();
  return 0;
}
