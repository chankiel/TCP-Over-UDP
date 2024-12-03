#include "server.hpp"
#include <cstdlib> // For malloc and free

Server::Server(string myIP, int myPort) {
  connection = new TCPSocket(myIP, myPort);
  this->serverIP = myIP;
  this->serverPort = myPort;
}

void Server::handleMessage(void *buffer) {
  Segment *segment = static_cast<Segment *>(buffer);

  if (segment->flags.syn == 1 && segment->flags.ack == 0) {
    std::cout << "Received SYN. Sending SYN-ACK..." << std::endl;
    Segment synAckSegment = synAck(0);
    connection->send(connection->getIP(), connection->getPort(), &synAckSegment,
                     sizeof(synAckSegment));
  } else if (segment->flags.cwr & ACK_FLAG) {
    std::cout << "Received ACK. Handshake completed!" << std::endl;
  }
}

void Server::startServer() {
  connection->listen(serverIP, serverPort);
  std::cout << "Server listening on port " << serverPort << std::endl;

  void *buffer = malloc(sizeof(Segment));
  while (true) {
    int bytesReceived = connection->ambil(buffer, sizeof(Segment));
    if (bytesReceived > 0) {
      handleMessage(buffer);
    }
  }

  free(buffer);
  connection->close();
}

int main() {
  string myIP = "0.0.0.0";
  int myPort = 8080;
  Server server(myIP, myPort);
  server.startServer();
  return 0;
}
