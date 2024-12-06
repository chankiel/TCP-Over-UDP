#include "client.hpp"
#include "tools.hpp"
#include "../Socket/socket.hpp"
#include "../tools/tools.hpp"
#include <cstdlib> // For malloc and free
#include <random>

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
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 4294967295);

  uint32_t r_seq_num = dist(gen);

  commandLine('i', "Sender Program’s Three Way Handshake\n");

  Segment synSegment = syn(r_seq_num);

  for (int i = 0; i < 10; i++)
  {
    try {
      // Send syn?
      connection->sendSegment(synSegment, serverIp_, serverPort_);
      connection->setSocketState(TCPState::SYN_SENT);

      commandLine('i', "[Established] [Seg 1] [S=" + std::to_string(r_seq_num) + "] Sending SYN request to " + serverIp_ + ":" + std::to_string(serverPort_) + "\n");

      // Wait syn-ack?
      Message result = connection->consumeBuffer(serverIp_, serverPort_, 0, r_seq_num + 1, SYN_ACK_FLAG, 10);
      Segment synAckSegment = result.segment;

      connection->setSocketState(TCPState::ESTABLISHED);

      commandLine('~', "[Established] Waiting for segments to be ACKed\n");
      commandLine('i', "[Established] [Seg 1] [S=" + std::to_string(synAckSegment.seqNum) + "] [A=" + std::to_string(synAckSegment.ackNum) + "] Received SYN-ACK request from " + synAckSegment.ip + ":" + std::to_string(synAckSegment.port) + "\n");

      // Send ack?
      uint32_t ackNum = synAckSegment.seqNum + 1;
      Segment ackSegment = ack(r_seq_num + 1, ackNum);
      connection->sendSegment(ackSegment, serverIp_, serverPort_);
      commandLine('i', "[Established] [Seg 2] [A=" + std::to_string(ackNum) + "] Sending ACK request to " + serverIp_ + ":" + std::to_string(serverPort_) + "\n");
      commandLine('i', "[Established] [Seg 2] [A=" + std::to_string(ackNum) + "] Sent\n");
      commandLine('~', "Ready to receive input from " + serverIp_ + ":" + std::to_string(serverPort_) + "\n");
      return;

    } catch (const std::exception &e) {
      commandLine('e', "[Handshake] Attempt failed: " + std::string(e.what()) + "\n");
    }
  }
  commandLine('e', "[Handshake] Failed after 10 retries\n");
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
