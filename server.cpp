#include "server.hpp"
#include "tools.hpp"
#include <cstdlib> // For malloc and free

Server::Server(string myIP, int myPort) {
  connection = new TCPSocket(myIP, myPort);
  this->serverIP = myIP;
  this->serverPort = myPort;
}

void Server::handleMessage(void *buffer) {
  Segment *segment = static_cast<Segment *>(buffer);

  // SYN
  if (segment->flags.syn == 1 && segment->flags.ack == 0) {
    Segment synAckSegment = synAck(0);
    commandLine('i', "[Handshake] [S=" + std::to_string(synAckSegment.seqNum) +
                         "] Received SYN request from " + connection->getIP() +
                         ":" + std::to_string(connection->getPort()) + "\n");
    synAckSegment.ackNum = synAckSegment.seqNum;
    synAckSegment.seqNum += sizeof(synAckSegment);
    commandLine('i', "[Handshake] [S=" + std::to_string(synAckSegment.seqNum) +
                         "] [A=" + std::to_string(synAckSegment.ackNum) +
                         "] Sending SYN-ACK request\n");
    connection->send(connection->getIP(), connection->getPort(), &synAckSegment,
                     sizeof(synAckSegment));
  } else if (segment->flags.syn == 0 && segment->flags.ack == 1) {
    commandLine('+', "[Handshake] [A=" + std::to_string(segment->ackNum) +
                         "] Received ACK request\n");
    commandLine('i', "Handshake complete. Ready to receive input.\n");
  }

  // FIN
  else if (segment->flags.fin == 1 && segment->flags.ack == 0) {
    commandLine('i', "[Close] Received FIN from " + connection->getIP() + ":" +
                         std::to_string(connection->getPort()) + "\n");

    // ACK
    Segment ackSegment = ack(0, segment->seqNum + 1);
    connection->send(connection->getIP(), connection->getPort(), &ackSegment,
                     sizeof(ackSegment));
    commandLine('i', "[Close] Sent ACK for FIN\n");

    // FIN
    Segment finSegment = fin();
    connection->send(connection->getIP(), connection->getPort(), &finSegment,
                     sizeof(finSegment));
    commandLine('i', "[Close] Sent FIN to close connection\n");
  } else if (segment->flags.fin == 0 && segment->flags.ack == 1) {
    commandLine('+', "[Close] Received ACK for FIN. Connection closed.\n");
    connection->close();
  }
}

int main_server() {
  string myIP = "0.0.0.0";
  int myPort = 8080;
  Server server(myIP, myPort);
  server.startServer();
  return 0;
}
