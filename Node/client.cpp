#include "client.hpp"
#include <stdexcept>
#include <string>
#include "../Socket/socket.hpp"
#include "../tools/tools.hpp"
#include <cstdlib> // For malloc and free
#include <random>
int CLIENT_BROADCAST_TIMEOUT = 3; // temporary
int CLIENT_MAX_TRY = 5;

ConnectionResult Client::findBroadcast(string dest_ip, uint16_t dest_port)
{
  for (int i = 0; i < CLIENT_MAX_TRY; i++)
  {
    try
    {
      Segment temp = createSegment("AWD",port,dest_port);

      connection->sendSegment(temp, dest_ip, dest_port);
      commandLine('i', "Sending Broadcast\n");
      Message answer =
          connection->consumeBuffer("", 0, 0, 0, 255, CLIENT_BROADCAST_TIMEOUT);
      commandLine('i', "Someone received the broadcast\n");
      return ConnectionResult(1, answer.ip, answer.port, answer.segment.seqNum,
                              answer.segment.ackNum);
    }
    catch (const std::runtime_error &e)
    {
      commandLine('x', "Timeout " + std::to_string(i + 1) + "\n");
      continue;
    }
  }
  return ConnectionResult(false, 0, 0, 0, 0);
}

ConnectionResult Client::startHandshake(string dest_ip, uint16_t dest_port)
{
  uint32_t r_seq_num = generateRandomNumber(10, 4294967295);

  commandLine('i', "Sender Program's Three Way Handshake\n");

  Segment synSegment = syn(r_seq_num);

  for (int i = 0; i < 10; i++)
  {
    try
    {
      // Send syn?
      connection->sendSegment(synSegment, dest_ip, dest_port);
      connection->setSocketState(TCPState::SYN_SENT);

      commandLine('i', "[Established] [Seg 1] [S=" + std::to_string(r_seq_num) + "] Sending SYN request to " + dest_ip + ":" + std::to_string(dest_port) + "\n");

      // Wait syn-ack?
      Message result = connection->consumeBuffer(dest_ip, dest_port, 0, r_seq_num + 1, SYN_ACK_FLAG, 10);
      Segment synAckSegment = result.segment;

      connection->setSocketState(TCPState::ESTABLISHED);

      commandLine('~', "[Established] Waiting for segments to be ACKed\n");
      commandLine('i', "[Established] [Seg 1] [S=" + std::to_string(synAckSegment.seqNum) + "] [A=" + std::to_string(synAckSegment.ackNum) + "] Received SYN-ACK request from " + result.ip + ":" + std::to_string(result.port) + "\n");

      // Send ack?
      uint32_t ackNum = synAckSegment.seqNum + 1;
      Segment ackSegment = ack(r_seq_num + 1, ackNum);
      connection->sendSegment(ackSegment, dest_ip, dest_port);
      commandLine('i', "[Established] [Seg 2] [A=" + std::to_string(ackNum) + "] Sending ACK request to " + dest_ip + ":" + std::to_string(dest_port) + "\n");
      commandLine('i', "[Established] [Seg 2] [A=" + std::to_string(ackNum) + "] Sent\n");
      commandLine('~', "Ready to receive input from " + dest_ip + ":" + std::to_string(dest_port) + "\n");
      return ConnectionResult(true, dest_ip, dest_port, synAckSegment.seqNum, synAckSegment.ackNum);
    }
    catch (const std::exception &e)
    {
      commandLine('e', "[Handshake] Attempt failed: " + std::string(e.what()) + "\n");
    }
  }
  commandLine('e', "[Handshake] Failed after 10 retries\n");
  return ConnectionResult(false, dest_ip, dest_port, 0, 0);
}

void Client::run()
{
  connection->listen();
  connection->startListening();
  // Segment ackSegment = ack(0, 0);
  // connection->sendSegment(ackSegment, "127.0.0.1", 8081);
  ConnectionResult statusBroadcast = findBroadcast("127.0.0.1", 8081);
  if (statusBroadcast.success)
  {
    std::cout << "SUCCESS" << std::endl;
  }
  else
  {
    std::cout << "FALSE" << std::endl;
  }
}