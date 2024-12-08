#include "client.hpp"
#include "../Socket/socket.hpp"
#include "../tools/tools.hpp"
#include <cstdlib> // For malloc and free
#include <pthread.h>
#include <random>
#include <stdexcept>
#include <string>
#include "../tools/fileReceiver.hpp"

int CLIENT_BROADCAST_TIMEOUT = 12; // temporary
int CLIENT_COMMON_TIMEOUT = 12;    // temporary
int CLIENT_MAX_TRY = 10;

ConnectionResult Client::findBroadcast(string dest_ip, uint16_t dest_port)
{
  connection->setBroadcast();

  std::cout << dest_ip << " " << dest_port << std::endl;
  for (int i = 0; i < CLIENT_MAX_TRY; i++)
  {
    try
    {
      // Segment temp = createSegment("AWD", port, dest_port);
      Segment temp = broad();
      updateChecksum(temp);

      // auto *buffer = new uint8_t[temp.payloadSize + 24];
      // encodeSegment(temp, buffer);
      // std::cout << "client checksum update 1: " << temp.checksum <<
      // std::endl; updateChecksum(temp); std::cout << "client checksum update
      // 2: " << temp.checksum << std::endl; printSegment(temp); temp.checksum =
      // calculateChecksum(temp);

      // std::cout << "client checksum calc 1: " << calculateChecksum(temp)
      //           << std::endl;
      // std::cout << "client checksum calc 2: " << calculateChecksum(temp)
      //           << std::endl;

      // temp = updateChecksum(temp); std::cout << "client checksum
      // update 2: " << temp.checksum << std::endl; printSegment(temp);

      std::cout << "find broadcast checksum " << temp.checksum << std::endl;
      connection->sendSegment(temp, dest_ip, dest_port);
      commandLine('i', "Sending Broadcast");
      Message answer =
          connection->consumeBuffer("", 0, 0, 0, 255, CLIENT_BROADCAST_TIMEOUT);
      commandLine('i', "Someone received the broadcast");
      return ConnectionResult(true, answer.ip, answer.port,
                              answer.segment.seqNum, answer.segment.ackNum);
    }
    catch (const std::runtime_error &e)
    {
      commandLine('x', "Timeout " + std::to_string(i + 1));
      continue;
    }
  }
  return ConnectionResult(false, 0, 0, 0, 0);
}

ConnectionResult Client::respondFin(string dest_ip, uint16_t dest_port,
                                    uint32_t seqNum, uint32_t ackNum)
{
  Message rec_fin = connection->consumeBuffer(
      dest_ip, dest_port, 0, seqNum + 1, FIN_FLAG, CLIENT_COMMON_TIMEOUT);
  commandLine('+', "[Closing] [S=" + to_string(rec_fin.segment.seqNum) +
                       "] [A=" + to_string(rec_fin.segment.ackNum) +
                       "] Received FIN request from  " + dest_ip +
                       to_string(dest_port));
  // Send ACK
  Segment ackSeg = ack(seqNum + 1, rec_fin.segment.seqNum + 1);
  updateChecksum(ackSeg);

  connection->sendSegment(ackSeg, dest_ip, dest_port);
  commandLine('i', "[Closing] [S=" + to_string(ackSeg.seqNum) +
                       "] [A=" + to_string(ackSeg.ackNum) +
                       "] Send ACK request from  " + dest_ip +
                       to_string(dest_port));

  for (int i = 0; i < CLIENT_MAX_TRY; i++)
  {
    try
    {
      // Send FIN
      Segment finSeg = fin(seqNum + 2, rec_fin.segment.seqNum + 1);
      updateChecksum(finSeg);
      // finSeg.checksum = calculateChecksum(finSeg);
      connection->sendSegment(finSeg, dest_ip, dest_port);
      commandLine('i', "[Closing] [S=" + to_string(finSeg.seqNum) +
                           "] [A=" + to_string(finSeg.ackNum) +
                           "] Send FIN request from  " + dest_ip +
                           to_string(dest_port));

      // REC ACK
      Message answer_fin =
          connection->consumeBuffer(dest_ip, dest_port, 0, finSeg.seqNum + 1,
                                    ACK_FLAG, CLIENT_COMMON_TIMEOUT);
      commandLine('+', "[Closing] [S=" + to_string(answer_fin.segment.seqNum) +
                           "] [A=" + to_string(answer_fin.segment.ackNum) +
                           "] Received FIN request from  " + dest_ip +
                           to_string(dest_port));

      commandLine('i', "Connection Closed");

      return ConnectionResult(true, dest_ip, dest_port, 0, 0);
    }
    catch (const std::runtime_error &e)
    {
      commandLine('x', "Timeout " + std::to_string(i + 1));
    }
  }
  return ConnectionResult(false, dest_ip, dest_port, 0, 0);
}

ConnectionResult Client::startHandshake(string dest_ip, uint16_t dest_port)
{
  uint32_t r_seq_num = generateRandomNumber(10, 4294967295);

  commandLine('i', "Sender Program's Three Way Handshake");

  Segment synSegment = syn(r_seq_num);
  updateChecksum(synSegment);

  for (int i = 0; i < 10; i++)
  {
    try
    {
      // Send syn?
      connection->sendSegment(synSegment, dest_ip, dest_port);
      connection->setStatus(TCPStatusEnum::SYN_SENT);

      commandLine('i', "[Handshake] [S=" + std::to_string(r_seq_num) +
                           "] Sending SYN request to " + dest_ip + ":" +
                           std::to_string(dest_port));

      // Wait syn-ack?
      Message result = connection->consumeBuffer(
          dest_ip, dest_port, 0, r_seq_num + 1, SYN_ACK_FLAG, 10);
      commandLine('i',
                  "[Handshake] [S=" + std::to_string(result.segment.seqNum) +
                      "] [A=" + std::to_string(result.segment.ackNum) +
                      "] Received SYN-ACK request to " + dest_ip + ":" +
                      std::to_string(dest_port));

      // Send ack?
      uint32_t ackNum = result.segment.seqNum + 1;
      Segment ackSegment = ack(r_seq_num + 1, ackNum);
      updateChecksum(ackSegment);

      // ackSegment.checksum = calculateChecksum(ackSegment);
      connection->sendSegment(ackSegment, dest_ip, dest_port);
      commandLine('i', "[Handshake] [S=" + std::to_string(ackSegment.seqNum) +
                           "] [A=" + std::to_string(ackSegment.ackNum) +
                           "] Sending SYN-ACK request to " + dest_ip + ":" +
                           std::to_string(dest_port));
      commandLine('~', "Ready to receive input from " + dest_ip + ":" +
                           std::to_string(dest_port));
      return ConnectionResult(true, dest_ip, dest_port, ackSegment.seqNum,
                              ackSegment.ackNum);
    }
    catch (const std::exception &e)
    {
      commandLine('e', "[Handshake] Attempt failed: " + std::string(e.what()));
    }
  }
  commandLine('e', "[Handshake] Failed after 10 retries");
  return ConnectionResult(false, dest_ip, dest_port, 0, 0);
}

void Client::run()
{
  connection->listen();
  connection->startListening();

  ConnectionResult statusBroadcast = findBroadcast("255.255.255.255", serverPort);
  if (!statusBroadcast.success)
  {
    std::cerr << "Error: Broadcast failed." << std::endl;
    return;
  }

  ConnectionResult statusHandshake = startHandshake(statusBroadcast.ip, statusBroadcast.port);
  if (!statusHandshake.success)
  {
    std::cerr << "Error: Handshake failed." << std::endl;
    return;
  }

  vector<Segment> res;
  ConnectionResult statusReceive = connection->receiveBackN(
      res, statusBroadcast.ip, statusBroadcast.port, statusHandshake.seqNum + 1);
  if (!statusReceive.success)
  {
    std::cerr << "Error: Receiving response failed." << std::endl;
    return;
  }

  ConnectionResult statusFin = respondFin(
      statusBroadcast.ip, statusBroadcast.port, statusHandshake.seqNum, statusHandshake.ackNum);
  if (!statusFin.success)
  {
    std::cerr << "Error: FIN response failed." << std::endl;
    return;
  }

  if (res.back().flags.ece == 1)
  {
    std::string filename(reinterpret_cast<char *>(res.back().payload), res.back().payloadSize);
    res.pop_back();
    std::string result = connection->concatenatePayloads(res);
    convertFromStrToFile(filename, result);
  }
  else
  {
    std::string result = connection->concatenatePayloads(res);
    std::cout << "Result string: " << result << std::endl;
  }
}
