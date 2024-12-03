#include "server.hpp"
#include <cstdlib> // For malloc and free

Server::Server(int port) {
  connection = new TCPSocket(clientIp_, port);
  port_ = port;
}

void Server::handleMessage(void *buffer) {
  Segment *segment = static_cast<Segment *>(buffer);

  if (segment->flags.syn == 1 && segment->flags.ack == 0) {
    std::cout << "Received SYN. Sending SYN-ACK..." << std::endl;
    Segment synAckSegment = synAck(0);
    connection->send(clientIp_, clientPort_, &synAckSegment,
                     sizeof(synAckSegment));
  } else if (segment->flags.cwr & ACK_FLAG) {
    std::cout << "Received ACK. Handshake completed!" << std::endl;
  }
}

void Server::startServer() {
  connection->listen(port_);
  std::cout << "Server listening on port " << port_ << std::endl;

  void *buffer = malloc(sizeof(Segment));
  while (true) {
    int bytesReceived = connection->ambil(buffer, sizeof(Segment));
    if (bytesReceived > 0) {
      handleMessage(buffer);
    }
  }
  free(buffer);
}

private:
int port_;
std::string clientIp_ = "127.0.0.1"; // For simplicity, assuming a local client.
int clientPort_ = 8081;              // Example client port.
}
;

int main() {
  Server server(8080);
  server.startServer();
  return 0;
}
