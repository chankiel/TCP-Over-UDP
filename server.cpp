#include "node.hpp"
#include <cstring>
#include <iostream>

class Server : public Node {
public:
  Server(int port) {
    connection = new TCPSocket(8080);
    port_ = port;
    struct sockaddr_in *servAddr, *cliAddr;
  }

  void handleMessage(void *buffer) override {
    Segment *segment = static_cast<Segment *>(buffer);
    if (segment->flags.syn == 1 && segment->flags.ack == 0) {
      std::cout << "Received SYN. Sending SYN-ACK..." << std::endl;
      Segment synAckSegment = synAck(0);
      connection->send(clientIp_, clientPort_, &synAckSegment,
                       sizeof(synAckSegment), 0);
    } else if (segment->flags.syn == 0 && segment->flags.ack == 1) {
      std::cout << "Received ACK. Handshake completed!" << std::endl;
    }
  }

  void startServer() {
    connection->listen();
    std::cout << "Server listening on port " << port_ << std::endl;

    void *buffer = malloc(sizeof(Segment));

    while (true) {
      int bytesReceived = connection->ambil(buffer, sizeof(Segment), 0);
      std::cout << "listening: " << bytesReceived << std::endl;
      if (bytesReceived > 0) {
        handleMessage(buffer);
      }
    }
    free(buffer);
  }

private:
  int port_;
  std::string clientIp_ = "0.0.0.0"; // For simplicity, assuming a local client.
  int clientPort_ = 8080;            // Example client port.
};

int main() {
  Server server(8080);
  server.startServer();
  return 0;
}
