#include "server.hpp"
#include "../tools/tools.hpp"
#include <stdexcept>
#include <string>

int SERVER_BROADCAST_TIMEOUT = 10; // temporary
int SERVER_COMMON_TIMEOUT = 12;    // temporary
int SERVER_MAX_TRY = 10;

ConnectionResult Server::respondHandshake(string dest_ip, uint16_t dest_port)
{
  for(int i=0;i<SERVER_MAX_TRY;i++)
  {
    try
    {
      Message sync_message =
          connection->consumeBuffer("", 0, 0, 0, SYN_FLAG, 10);
      connection->setStatus(TCPStatusEnum::SYN_RECEIVED);
      std::string destIP = sync_message.ip;
      uint16_t destPort = sync_message.port;
      uint32_t sequence_num_first = sync_message.segment.seqNum;

      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + std::to_string(sequence_num_first) +
                   "] Received SYN request from " + dest_ip + ":" +
                   std::to_string(destPort));

      // Sending SYN-ACK Request
      uint32_t sequence_num_second = generateRandomNumber(1, 1000);
      uint32_t ack_num_second = sequence_num_first + 1;

      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + std::to_string(sequence_num_second) +
                   "] [A=" + std::to_string(ack_num_second) +
                   "] Sending SYN-ACK request to " + dest_ip + ":" +
                   std::to_string(destPort));
      Segment synSeg = synAck(sequence_num_second, ack_num_second);
      updateChecksum(synSeg);
      connection->sendSegment(synSeg, dest_ip, dest_port);
      connection->setStatus(TCPStatusEnum::SYN_SENT);

      // Received ACK Request
      Message ack_message =
          connection->consumeBuffer(destIP, destPort, 0, 0, ACK_FLAG);
      connection->setStatus(TCPStatusEnum::ESTABLISHED);
      uint32_t ack_num_third = ack_message.segment.ackNum;
      uint32_t seq_num_third = ack_message.segment.seqNum;
      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [A=" + std::to_string(ack_num_third) +
                   "] Received ACK request from " + dest_ip + ":" +
                   std::to_string(destPort));

      // Check Sequence and ACK validity
      if (ack_num_third == sequence_num_second + 1)
      {
        commandLine('i', "Sending input to " + dest_ip + ":" +
                             std::to_string(destPort));
        return ConnectionResult(true, destIP, destPort, sequence_num_second + 1,
                                seq_num_third + 1);
      }
    }
    catch (const std::exception &e)
    {
      cout << ERROR << brackets("TIMEOUT") + "Restarting Handshake" + brackets("ATTEMPT-" + std::to_string(i + 1))<<std::endl;
    }
  }
  return ConnectionResult(false, dest_ip, dest_port, 0, 0);
}

ConnectionResult Server::listenBroadcast()
{
  std::cout<<std::endl;
  commandLine('i', "Listening to the broadcast port for clients.");
    try
    {
      Message answer =
          connection->consumeBuffer("", 0, 0, 0, 0, SERVER_BROADCAST_TIMEOUT);
      connection->setStatus(TCPStatusEnum::LISTENING);
      commandLine('+', "Received Broadcast Message");
      Segment temp = accBroad();
      updateChecksum(temp);
      connection->sendSegment(temp, answer.ip, answer.port);
      return ConnectionResult(true, answer.ip, answer.port,
                              answer.segment.seqNum, answer.segment.ackNum);
    }
    catch (const std::runtime_error &e)
    {
      return ConnectionResult(false, "", 0, 0, 0);
    }
  }

ConnectionResult Server::startFin(string dest_ip, uint16_t dest_port,
                                  uint32_t seqNum, uint32_t ackNum)
{
  for (int i = 0; i < SERVER_MAX_TRY; i++)
  {
    try
    {
      // Send Fin
      connection->setStatus(TCPStatusEnum::CLOSE_WAIT);
      Segment finSeg = fin(seqNum + 1, ackNum);
      updateChecksum(finSeg);
      connection->sendSegment(finSeg, dest_ip, dest_port);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(finSeg.seqNum) + "] [A=" +
                   to_string(finSeg.ackNum) + "] Sending FIN request to " +
                   dest_ip + ":" + to_string(dest_port));
      // REC ACK
      Message answer_fin =
          connection->consumeBuffer(dest_ip, dest_port, 0, finSeg.seqNum + 1,
                                    ACK_FLAG, SERVER_COMMON_TIMEOUT);
      commandLine(
          '+', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(answer_fin.segment.seqNum) +
                   "] [A=" + to_string(answer_fin.segment.ackNum) +
                   "] Received ACK request from  " + dest_ip +
                   to_string(dest_port));
      // REC FIN
      connection->setStatus(TCPStatusEnum::LAST_ACK);
      Message fin2 =
          connection->consumeBuffer(dest_ip, dest_port, 0, finSeg.seqNum + 1,
                                    FIN_FLAG, SERVER_COMMON_TIMEOUT);
      commandLine(
          '+', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(fin2.segment.seqNum) +
                   "] [A=" + to_string(fin2.segment.ackNum) +
                   "] Received FIN request from  " + dest_ip +
                   to_string(dest_port));

      // Send ACK
      connection->setStatus(TCPStatusEnum::CLOSED);
      Segment ackSeg = ack(seqNum + 2, fin2.segment.seqNum + 1);
      updateChecksum(ackSeg);
      connection->sendSegment(ackSeg, dest_ip, dest_port);

      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(ackSeg.seqNum) + "] [A=" +
                   to_string(ackSeg.ackNum) + "] Sending FIN request to " +
                   dest_ip + ":" + to_string(dest_port));

      commandLine('i', "Connection Closed");
      return ConnectionResult(true, dest_ip, dest_port, 0, 0);
    }
    catch (const std::runtime_error &e)
    {
      cout<< ERROR<< brackets("TIMEOUT") + "No respond for FIN request. Restarting sending for FIN to Client"+ brackets("ATTEMPT-"+std::to_string(i + 1))<<std::endl;
      continue;
    }
  }
  return ConnectionResult(false, dest_ip, dest_port, 0, 0);
}

void Server::run()
{
  connection->listen();
  connection->startListening();

  while (true)
  {
    connection->setStatus(TCPStatusEnum::LISTENING);
    ConnectionResult statusBroadcast = listenBroadcast();
    if (!statusBroadcast.success)
    {
      std::cerr << ERROR<<" Failed to receive broadcast. Restarting Server." << std::endl;
      continue;
    }

    ConnectionResult statusHandshake = respondHandshake(statusBroadcast.ip, statusBroadcast.port);
    if (!statusHandshake.success)
    {
      std::cerr << ERROR<<" Handshake response failed. Restarting Server." << std::endl;
      continue;
    }

    bool isFile = true;
    std::string fileFullName;
    if (fileEx == "-1")
    {
      isFile = false;
    }
    else
    {
      fileFullName = fileEx.empty() ? fileName : fileName + "." + fileEx;
    }

    ConnectionResult statusSend = connection->sendBackN(
        reinterpret_cast<uint8_t *>(item.data()),
        static_cast<uint32_t>(item.length()),
        statusBroadcast.ip,
        statusBroadcast.port,
        statusHandshake.ackNum,
        isFile,
        fileFullName);
    if (!statusSend.success)
    {
      std::cerr << ERROR<<" Sending data failed. Restarting Server." << std::endl;
      continue;
    }
    cout << OUT << " Start waiting for Client if Server finished first." << endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    ConnectionResult statusFin = startFin(
        statusBroadcast.ip,
        statusBroadcast.port,
        statusHandshake.seqNum,
        statusHandshake.ackNum);
    if (!statusFin.success)
    {
      std::cerr << ERROR<<" FIN process failed. Restarting Server." << std::endl;
    }
  }
}
