#include "client.hpp"
#include <cstdlib> // For malloc and free

Client::Client(const std::string &serverIp, int serverPort) {
  connection = new TCPSocket(serverIp, serverPort);
  serverIp_ = serverIp;
  serverPort_ = serverPort;
}

void Client::handleMessage(void *buffer) {
  Segment *segment = static_cast<Segment *>(buffer);

  if (segment->flags.syn == 1 && segment->flags.ack == 1) {
    std::cout << "Received SYN-ACK. Sending ACK..." << std::endl;
    Segment ackSegment = ack(0, 1);
    connection->send(serverIp_, serverPort_, &ackSegment, sizeof(ackSegment));
    std::cout << "Handshake completed!" << std::endl;
  }
}

void Client::startHandshake() {
  connection->listen(8081);
  Segment synSegment = syn(0);
  connection->send(serverIp_, serverPort_, &synSegment, sizeof(synSegment));
  std::cout << "Sent SYN packet to server." << std::endl;

  void *buffer = malloc(sizeof(Segment));
  std::cout << "Before receiving data in client" << std::endl;
  connection->ambil(buffer, sizeof(Segment));
  std::cout << "After receiving data in client" << std::endl;
  handleMessage(buffer);
  free(buffer);
}
