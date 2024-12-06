#include "server.hpp"
#include "../tools/tools.hpp"
#include <stdexcept>

int SERVER_BROADCAST_TIMEOUT = 12; // temporary
int SERVER_COMMON_TIMEOUT = 12;    // temporary
int SERVER_MAX_TRY = 10;

ConnectionResult Server::respondHandshake(string dest_ip, uint16_t dest_port)
{
  int retries = SERVER_MAX_TRY;
  while (retries-- > 0)
  {
    try
    {
      // Get all the possible buffer
      Message sync_message = connection->consumeBuffer("", 0, 0, 0, SYN_FLAG, 10);

      std::string destIP = sync_message.ip;
      u_int16_t destPort = sync_message.port;
      u_int32_t sequence_num_first = sync_message.segment.seqNum;

      commandLine('i', "[Handshake] [S=" + std::to_string(sequence_num_first) + "] Received SYN request from " + dest_ip + ":" + std::to_string(destPort));

      // Sending SYN-ACK Request
      uint32_t sequence_num_second = generateRandomNumber(1, 1000);
      u_int32_t ack_num_second = sequence_num_first + 1;

      commandLine('i', "[Handshake] [S=" + std::to_string(sequence_num_second) + "] [A=" + std::to_string(ack_num_second) + "] Sending SYN-ACK request to " + dest_ip + ":" + std::to_string(destPort));
      Segment synSeg = synAck(sequence_num_second, ack_num_second);
      // std::cout << synSeg.seqNum << " " << synSeg.ackNum << std::endl;
      connection->sendSegment(synSeg, dest_ip, dest_port);
      connection->setSocketState(TCPState::SYN_SENT);

      // Received ACK Request
      Message ack_message = connection->consumeBuffer(destIP, destPort, 0, 0, ACK_FLAG);
      u_int32_t ack_num_third = ack_message.segment.ackNum;
      u_int32_t seq_num_third = ack_message.segment.seqNum;
      commandLine('i', "[Handshake] [A=" + std::to_string(ack_num_third) + "] Received ACK request from " + dest_ip + ":" + std::to_string(destPort));

      // Check Sequence and ACK validity
      if (ack_num_third == sequence_num_second + 1)
      {
        commandLine('i', "Sending input to " + dest_ip + ":" + std::to_string(destPort));
        return ConnectionResult(true, destIP, destPort, seq_num_third, ack_num_third);
      }
    }
    catch (const std::exception &e)
    {
      commandLine('!', "[ERROR] [HANDSHAKE] " + std::string(e.what()));
    }
  }

  commandLine('!', "[ERROR] [HANDSHAKE] Handshake failed after " + std::to_string(retries) + " retries");
  return ConnectionResult(false, dest_ip, dest_port, 0, 0);
}

ConnectionResult Server::listenBroadcast()
{
  // connection->setBroadcast();
  for (int i = 0; i < SERVER_MAX_TRY; i++)
  {
    try
    {
      Message answer =
          connection->consumeBuffer("", 0, 0, 0, 0, SERVER_BROADCAST_TIMEOUT);
      commandLine('+', "Received Broadcast Message\n");
      Segment temp = accBroad();
      connection->sendSegment(temp, answer.ip, answer.port);
      return ConnectionResult(true, answer.ip, answer.port, answer.segment.seqNum,
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

ConnectionResult Server::respondFin(string dest_ip, uint16_t dest_port,
                                    uint32_t seqNum, uint32_t ackNum) {
  for (int i = 0; i < SERVER_MAX_TRY; i++) {
    try {
      // OLD
      // // Rec FIN
      // std::cout<<seqNum<<" "<<ackNum<<endl;
      // Message rec_fin = connection->consumeBuffer(dest_ip, dest_port, 0, ackNum,
      //                                             FIN_FLAG, SERVER_COMMON_TIMEOUT);
      // commandLine('+', "[Closing] Received FIN request from  " + dest_ip +
      //                      to_string(dest_port) + "\n");
      // // Send ACK
      // Segment ackSeg = ack(rec_fin.segment.ackNum, rec_fin.segment.seqNum+1);
      // connection->sendSegment(ackSeg, dest_ip, dest_port);
      // commandLine('i', "[Closing] Sending FIN request to " + dest_ip +
      //                      to_string(dest_port) + "\n");
      // // Send FIN
      // Segment finSeg = fin(rec_fin.segment.ackNum+1,rec_fin.segment.seqNum+1);
      // connection->sendSegment(finSeg, dest_ip, dest_port);
      // commandLine('i', "[Closing] Sending FIN request to " + dest_ip +
      //                      to_string(dest_port) + "\n");
      // // REC ACK
      // Message answer_fin = connection->consumeBuffer(
      //     dest_ip, dest_port, rec_fin.segment.seqNum+1, rec_fin.segment.ackNum+2, ACK_FLAG, SERVER_COMMON_TIMEOUT);
      // commandLine('+', "[Closing] Received ACK request from  " + dest_ip +
      //                      to_string(dest_port) + "\n");
      // commandLine('i', "Connection Closed\n");
      // return ConnectionResult(true, dest_ip, dest_port,0,0);
      
      // NEW
      // Rec FIN
      std::cout<<seqNum<<" "<<ackNum<<endl;
      Message rec_fin = connection->consumeBuffer(dest_ip, dest_port, 0, seqNum,
                                                  FIN_FLAG, SERVER_COMMON_TIMEOUT);
      commandLine('+', "[Closing] Received FIN request from  " + dest_ip +
                           to_string(dest_port) + "\n");
      // Send ACK
      Segment ackSeg = ack(rec_fin.segment.ackNum, rec_fin.segment.seqNum+1);
      connection->sendSegment(ackSeg, dest_ip, dest_port);
      commandLine('i', "[Closing] Sending FIN request to " + dest_ip +
                           to_string(dest_port) + "\n");
      // Send FIN
      Segment finSeg = fin(rec_fin.segment.ackNum+1,rec_fin.segment.seqNum+1);
      connection->sendSegment(finSeg, dest_ip, dest_port);
      commandLine('i', "[Closing] Sending FIN request to " + dest_ip +
                           to_string(dest_port) + "\n");
      // REC ACK
      Message answer_fin = connection->consumeBuffer(
          dest_ip, dest_port, rec_fin.segment.seqNum+1, rec_fin.segment.ackNum+2, ACK_FLAG, SERVER_COMMON_TIMEOUT);
      commandLine('+', "[Closing] Received ACK request from  " + dest_ip +
                           to_string(dest_port) + "\n");
      commandLine('i', "Connection Closed\n");
      return ConnectionResult(true, dest_ip, dest_port,0,0);
    } catch (const std::runtime_error &e)
    {
      commandLine('x', "Timeout " + std::to_string(i + 1) + "\n");
      continue;
    }
  }
  return ConnectionResult(false, dest_ip, dest_port,0,0);
}

void Server::run()
{
  connection->listen();
  connection->startListening();
  ConnectionResult statusBroadcast = listenBroadcast();
  ConnectionResult statusHandshake = respondHandshake(statusBroadcast.ip,statusBroadcast.port);
  // std::cout<<statusHandshake.seqNum<<" "<<statusHandshake.ackNum<<std::endl;
  ConnectionResult statusFin = respondFin(statusBroadcast.ip,statusBroadcast.port,statusHandshake.ackNum,statusHandshake.seqNum+1);
  if (statusBroadcast.success)
  {
    std::cout << "SUCCESS" << std::endl;
  }
  else
  {
    std::cout << "FALSE" << std::endl;
  }
}
