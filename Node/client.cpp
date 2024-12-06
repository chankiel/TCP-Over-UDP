#include "client.hpp"
#include <stdexcept>
#include <string>
#include "../Socket/socket.hpp"
#include "../tools/tools.hpp"
#include <cstdlib> // For malloc and free
#include <random>
int BROADCAST_TIMEOUT = 30; // temporary
int MAX_TRY = 10;

ConnectionResult Client::findBroadcast(string dest_ip, uint16_t dest_port) {
  for (int i = 0; i < MAX_TRY; i++) {
    try {
      Segment temp = broad();

      connection->sendSegment(temp, dest_ip, dest_port);
      commandLine('i', "Sending Broadcast\n");
      Message answer =
          connection->consumeBuffer("", 0, 0, 0, 255, BROADCAST_TIMEOUT);
      commandLine('i', "Someone received the broadcast\n");
      return ConnectionResult(1, answer.ip, answer.port, answer.segment.seqNum,
                              answer.segment.ackNum);
    } catch (runtime_error()) {
      commandLine('x', "Timeout " + std::to_string(i + 1) + "\n");
      continue;
    }
  }
  return ConnectionResult(-1, 0, 0, 0, 0);
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