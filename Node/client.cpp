#include "client.hpp"
#include "../Socket/socket.hpp"
#include "../tools/fileReceiver.hpp"
#include "../tools/tools.hpp"
#include <cstdint>
#include <cstdlib>
#include <pthread.h>
#include <random>
#include <stdexcept>
#include <string>

int CLIENT_BROADCAST_TIMEOUT = 12;
int CLIENT_COMMON_TIMEOUT = 12;
int CLIENT_MAX_TRY = 10;

ConnectionResult Client::findBroadcast(string dest_ip, uint16_t dest_port)
{
  connection->setBroadcast();
  for (int i = 0; i < CLIENT_MAX_TRY; i++)
  {
    try
    {
      Segment temp = broad();
      updateChecksum(temp);

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
      cout << ERROR << brackets("TIMEOUT") + "Restarting searching for Broadcast Server" + brackets("ATTEMPT-" + std::to_string(i + 1))<<std::endl;
    }
  }
  throw std::runtime_error("Broadcast failed. Terminating Client. Thank you!");
}

ConnectionResult Client::respondFin(string dest_ip, uint16_t dest_port,
                                    uint32_t seqNum, uint32_t ackNum, uint32_t recfin_seqnum)
{
  bool isFirst = true;
  uint32_t recfin_seq = recfin_seqnum;
  for (int i = 0; i < CLIENT_MAX_TRY; i++)
  {
    try
    {
      connection->setStatus(TCPStatusEnum::FIN_WAIT_1);
      if(!isFirst){
        Message rec_fin = connection->consumeBuffer(
            dest_ip, dest_port, 0, seqNum + 1, FIN_FLAG, CLIENT_COMMON_TIMEOUT);
        recfin_seq = rec_fin.segment.seqNum;
      }

      commandLine(
          '+', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(recfin_seq) +
                   "] [A=" + to_string(seqNum+1) +
                   "] Received FIN request from  " + dest_ip +
                   to_string(dest_port));
      // Send ACK
      connection->setStatus(TCPStatusEnum::FIN_WAIT_2);
      Segment ackSeg = ack(seqNum + 1, recfin_seq + 1);
      updateChecksum(ackSeg);
      connection->sendSegment(ackSeg, dest_ip, dest_port);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(ackSeg.seqNum) + "] [A=" +
                   to_string(ackSeg.ackNum) + "] Send ACK request from  " +
                   dest_ip + to_string(dest_port));
      // Send FIN
      connection->setStatus(TCPStatusEnum::TIME_WAIT);
      Segment finSeg = fin(seqNum + 2, recfin_seq + 1);
      updateChecksum(finSeg);
      connection->sendSegment(finSeg, dest_ip, dest_port);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(finSeg.seqNum) + "] [A=" +
                   to_string(finSeg.ackNum) + "] Send FIN request from  " +
                   dest_ip + to_string(dest_port));

      // REC ACK
      connection->setStatus(TCPStatusEnum::CLOSED);
      Message answer_fin =
          connection->consumeBuffer(dest_ip, dest_port, 0, finSeg.seqNum + 1,
                                    ACK_FLAG, CLIENT_COMMON_TIMEOUT);
      commandLine(
          '+', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(answer_fin.segment.seqNum) +
                   "] [A=" + to_string(answer_fin.segment.ackNum) +
                   "] Received FIN request from  " + dest_ip +
                   to_string(dest_port));

      commandLine('i', "Connection Closed");

      return ConnectionResult(true, dest_ip, dest_port, 0, 0);
    }
    catch (const std::runtime_error &e)
    {
      cout << ERROR << brackets("TIMEOUT") + "Restarting Process, Prepared to Respond for FIN" + brackets("ATTEMPT-" + std::to_string(i + 1))<<std::endl;
    }
  }
  throw std::runtime_error("Responding for Server's FIN Failed. Terminating Client. Thank you!");
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
      connection->sendSegment(synSegment, dest_ip, dest_port);
      connection->setStatus(TCPStatusEnum::SYN_SENT);

      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + std::to_string(r_seq_num) +
                   "] Sending SYN request to " + dest_ip + ":" +
                   std::to_string(dest_port));

      Message result = connection->consumeBuffer(
          dest_ip, dest_port, 0, r_seq_num + 1, SYN_ACK_FLAG, 10);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + std::to_string(result.segment.seqNum) +
                   "] [A=" + std::to_string(result.segment.ackNum) +
                   "] Received SYN-ACK request to " + dest_ip + ":" +
                   std::to_string(dest_port));

      uint32_t ackNum = result.segment.seqNum + 1;
      Segment ackSegment = ack(r_seq_num + 1, ackNum);
      updateChecksum(ackSegment);

      connection->sendSegment(ackSegment, dest_ip, dest_port);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + std::to_string(ackSegment.seqNum) +
                   "] [A=" + std::to_string(ackSegment.ackNum) +
                   "] Sending ACK request to " + dest_ip + ":" +
                   std::to_string(dest_port));
      commandLine('~', "Ready to receive input from " + dest_ip + ":" +
                           std::to_string(dest_port));
      connection->setStatus(TCPStatusEnum::ESTABLISHED);
      return ConnectionResult(true, dest_ip, dest_port, ackSegment.seqNum,
                              ackSegment.ackNum);
    }
    catch (const std::exception &e)
    {
      cout << ERROR << brackets("TIMEOUT") + "Restarting Handshake" + brackets("ATTEMPT-" + std::to_string(i + 1))<<std::endl;
    }
  }
  throw std::runtime_error("Handshake failed. Terminating Client. Thank you!");
}

void Client::run()
{
  connection->listen();
  connection->startListening();
  try{

    ConnectionResult statusBroadcast =
        findBroadcast("255.255.255.255", serverPort);

    ConnectionResult statusHandshake =
        startHandshake(statusBroadcast.ip, statusBroadcast.port);

    vector<Segment> res;
    ConnectionResult statusReceive =
        connection->receiveBackN(res, statusBroadcast.ip, statusBroadcast.port,
                                statusHandshake.seqNum + 1);

    ConnectionResult statusFin =
        respondFin(statusBroadcast.ip, statusBroadcast.port,
                  statusHandshake.seqNum, statusHandshake.ackNum, statusReceive.seqNum);

    if (res.back().flags.ece == 1)
    {
      std::string filename(reinterpret_cast<char *>(res.back().payload),
                          res.back().payloadSize);
      res.pop_back();
      std::string result = connection->concatenatePayloads(res);
      convertFromStrToFile(filename, result);
      std::cout << OUT << " Terminating Client. Thank you!" << std::endl;
    }
    else
    {
      std::string result = connection->concatenatePayloads(res);
      std::cout << OUT << " String received from Server. Result: " << std::endl;
      std::cout << OUT <<" "<< result << std::endl;
      std::cout << OUT << " Terminating Client. Thank you!" << std::endl;
      exit(0);
    }
  }catch (const std::runtime_error &e){
    commandLine('!', "[ERROR] " + brackets(status_strings[(int)connection->getStatus()]) + std::string(e.what()));
    exit(0);
  }
}
