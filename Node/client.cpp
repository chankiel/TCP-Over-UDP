#include "client.hpp"
#include "tools.hpp"
#include <cstdlib> // For malloc and free

void Client::handleMessage(void *buffer, int sizeBuffer) {
  Segment *segment = static_cast<Segment *>(buffer);

  // Handling SYN-ACK
  if (segment->flags.syn == 1 && segment->flags.ack == 1) {
    Segment ackSegment = ack(0, segment->seqNum + 1);
    commandLine('i', "[Handshake] [S=" + std::to_string(ackSegment.seqNum) +
                         "] [A=" + std::to_string(ackSegment.ackNum) +
                         "] Received SYN-ACK\n");
    connection->send(connection->getIP(), connection->getPort(), &ackSegment,
                     sizeof(ackSegment));
    commandLine('i', "[Handshake] Sent ACK. Connection established.\n");
  }

  // Handling FIN
  else if (segment->flags.fin == 1 && segment->flags.ack == 0) {
    commandLine('i', "[Close] Received FIN from " + connection->getIP() + ":" +
                         std::to_string(connection->getPort()) + "\n");

    // Send ACK
    Segment ackSegment = ack(0, segment->seqNum + 1);
    connection->send(connection->getIP(), connection->getPort(), &ackSegment,
                     sizeof(ackSegment));
    commandLine('i', "[Close] Sent ACK for FIN\n");

    // Send FIN
    Segment finSegment = fin();
    connection->send(connection->getIP(), connection->getPort(), &finSegment,
                     sizeof(finSegment));
    commandLine('i', "[Close] Sent FIN to close connection\n");
  } else if (segment->flags.fin == 0 && segment->flags.ack == 1) {
    commandLine('+', "[Close] Received ACK for FIN. Connection closed.\n");
    connection->close();
  }
}

void Client::closeConnection() {
  // Send FIN
  Segment finSegment = fin();
  commandLine('i', "[Close] Sending FIN to " + connection->getIP() + ":" +
                       std::to_string(connection->getPort()) + "\n");
  connection->send(connection->getIP(), connection->getPort(), &finSegment,
                   sizeof(finSegment));

  // Wait for ACK and FIN
  void *buffer = malloc(sizeof(Segment));
  connection->ambil(buffer, sizeof(Segment));
  handleMessage(buffer);
  free(buffer);
}

void Client::startHandshake() {
  int synchornization = 0;
  std::string ip_handshake = "0.0.0.0";
  int port_handshake = 8080;
  connection->listen(clientIP, clientPort);
  Segment synSegment = syn(synchornization);
  commandLine('i', "[Handshake] [S=" + std::to_string(synchornization) +
                       "] Sending SYN request to " + ip_handshake + ":" +
                       std::to_string(port_handshake) + "\n");
  connection->send(ip_handshake, port_handshake, &synSegment,
                   sizeof(synSegment));

  void *buffer = malloc(sizeof(Segment));
  connection->ambil(buffer, sizeof(Segment));
  handleMessage(buffer);
  free(buffer);
}

// private:
//   std::string serverIp_;
//   int serverPort_;
// };

void Client::run(){
  cout<<"HEHE"<<endl;
}



int main_client() {
  // string testIP = "127.0.0.1";
  // int testPort = 8081;
  // Client client(testIP, testPort);

  // client.startHandshake();

  // // Simulate data exchange...
  // // std::this_thread::sleep_for(std::chrono::seconds(2));

  // client.closeConnection();
  return 0;
}
