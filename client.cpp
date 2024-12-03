#include "client.hpp"
#include <cstdlib> // For malloc and free

Client::Client(string myIP, int myport) {
  this->clientPort = myport;
  this->clientIP = myIP;
  connection = new TCPSocket(myIP, myport);
}

void Client::handleMessage(void *buffer) {
  Segment *segment = static_cast<Segment *>(buffer);

  if (segment->flags.syn == 1 && segment->flags.ack == 1) {
    std::cout << "Received SYN-ACK. Sending ACK..." << std::endl;
    Segment ackSegment = ack(0, 1);
    connection->send(connection->getIP(), connection->getPort(), &ackSegment,
                     sizeof(ackSegment));
    std::cout << "Handshake completed!" << std::endl;
  }
}

void Client::startHandshake() {
  connection->listen(clientIP, clientPort);
  Segment synSegment = syn(0);
  connection->send("0.0.0.0", 8080, &synSegment, sizeof(synSegment));
  std::cout << "Sent SYN packet to server." << std::endl;

  void *buffer = malloc(sizeof(Segment));
  connection->ambil(buffer, sizeof(Segment));
  handleMessage(buffer);
  free(buffer);
}

// private:
//   std::string serverIp_;
//   int serverPort_;
// };

int main() {
  string testIP = "127.0.0.1";
  int testPort = 8081;
  Client client(testIP, testPort);
  client.startHandshake();
  return 0;
}
