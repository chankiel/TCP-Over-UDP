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

  if (segment->flags.syn == 1 && segment->flags.ack == 0) {
    Segment synAckSegment = synAck(0);
    commandLine('i',"[Handshake] [S="+std::to_string(synAckSegment.seqNum)+"] Received SYN request from "+connection->getIP()+":"+std::to_string(connection->getPort())+"\n");
    synAckSegment.ackNum = synAckSegment.seqNum;
    synAckSegment.seqNum += sizeof(synAckSegment);
    commandLine(
      'i',
      "[Handshake] [S="+std::to_string(synAckSegment.seqNum)+"] [A="+std::to_string(synAckSegment.ackNum)+"] Sending SYN-ACK request from "+connection->getIP()+":"+std::to_string(connection->getPort())+"\n"
      );
    connection->send(connection->getIP(), connection->getPort(), &synAckSegment,
                     sizeof(synAckSegment));
  } else if (segment->flags.syn == 0 && segment->flags.ack == 1) {
    commandLine('+',"[Handshake] [A="+std::to_string(segment->ackNum)+"] Received ACK request from "+connection->getIP()+":"+std::to_string(connection->getPort())+"\n");
    commandLine('i', "Sending input to "+connection->getIP()+":"+std::to_string(connection->getPort())+"\n");
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

int main_server() {
  string myIP = "0.0.0.0";
  int myPort = 8080;
  Server server(myIP, myPort);
  server.startServer();
  return 0;
}
