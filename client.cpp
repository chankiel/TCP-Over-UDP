#include "client.hpp"
#include <cstdlib> // For malloc and free
#include "tools.hpp"

Client::Client(string myIP, int myport) {
  this->clientPort = myport;
  this->clientIP = myIP;
  connection = new TCPSocket(myIP, myport);
}

void Client::handleMessage(void *buffer) {
  Segment *segment = static_cast<Segment *>(buffer);

  if (segment->flags.syn == 1 && segment->flags.ack == 1) {
    Segment ackSegment = ack(0, 1);
    commandLine(
      'i',
      "[Handshake] [S="+std::to_string(ackSegment.seqNum)+"] [A="+std::to_string(ackSegment.ackNum)+"] Received SYN-ACK request from "+connection->getIP()+":"+std::to_string(connection->getPort())+"\n"
      );
    ackSegment.ackNum = ackSegment.seqNum+1;
    commandLine('i',"[Handshake] [A="+std::to_string(ackSegment.ackNum)+"] Sending ACK request to "+connection->getIP()+":"+std::to_string(connection->getPort())+"\n");
    connection->send(connection->getIP(), connection->getPort(), &ackSegment,
                     sizeof(ackSegment));
    commandLine('i', "Ready to receive input from "+connection->getIP()+":"+std::to_string(connection->getPort())+"\n");
  }
}

void Client::startHandshake() {
  int synchornization = 0;
  std::string ip_handshake = "0.0.0.0";
  int port_handshake = 8080;
  connection->listen(clientIP, clientPort);
  Segment synSegment = syn(synchornization);
  commandLine('i',"[Handshake] [S="+std::to_string(synchornization)+"] Sending SYN request to "+ip_handshake+":"+std::to_string(port_handshake)+"\n");
  connection->send(ip_handshake, port_handshake, &synSegment, sizeof(synSegment));

  void *buffer = malloc(sizeof(Segment));
  connection->ambil(buffer, sizeof(Segment));
  handleMessage(buffer);
  free(buffer);
}

// private:
//   std::string serverIp_;
//   int serverPort_;
// };

int main_client() {
  string testIP = "127.0.0.1";
  int testPort = 8081;
  Client client(testIP, testPort);
  client.startHandshake();
  return 0;
}
