#include "server.hpp"
#include "../tools/tools.hpp"
#include <stdexcept>
#include <string>

int SERVER_BROADCAST_TIMEOUT = 12; // temporary
int SERVER_COMMON_TIMEOUT = 12;    // temporary
int SERVER_MAX_TRY = 10;

ConnectionResult Server::respondHandshake(string dest_ip, uint16_t dest_port) {
  int retries = SERVER_MAX_TRY;
  while (retries-- > 0) {
    try {

      // Get all the possible buffer
      Message sync_message =
          connection->consumeBuffer("", 0, 0, 0, SYN_FLAG, 10);
      connection->setStatus(TCPStatusEnum::SYN_RECEIVED);
      std::string destIP = sync_message.ip;
      u_int16_t destPort = sync_message.port;
      u_int32_t sequence_num_first = sync_message.segment.seqNum;

      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + std::to_string(sequence_num_first) +
                   "] Received SYN request from " + dest_ip + ":" +
                   std::to_string(destPort));

      // Sending SYN-ACK Request
      uint32_t sequence_num_second = generateRandomNumber(1, 1000);
      u_int32_t ack_num_second = sequence_num_first + 1;

      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + std::to_string(sequence_num_second) +
                   "] [A=" + std::to_string(ack_num_second) +
                   "] Sending SYN-ACK request to " + dest_ip + ":" +
                   std::to_string(destPort));
      Segment synSeg = synAck(sequence_num_second, ack_num_second);
      updateChecksum(synSeg);
      // synSeg.checksum = calculateChecksum(synSeg);
      connection->sendSegment(synSeg, dest_ip, dest_port);
      connection->setStatus(TCPStatusEnum::SYN_SENT);

      // Received ACK Request
      Message ack_message =
          connection->consumeBuffer(destIP, destPort, 0, 0, ACK_FLAG);
      connection->setStatus(TCPStatusEnum::ESTABLISHED);
      u_int32_t ack_num_third = ack_message.segment.ackNum;
      u_int32_t seq_num_third = ack_message.segment.seqNum;
      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [A=" + std::to_string(ack_num_third) +
                   "] Received ACK request from " + dest_ip + ":" +
                   std::to_string(destPort));

      // Check Sequence and ACK validity
      if (ack_num_third == sequence_num_second + 1) {
        commandLine('i', "Sending input to " + dest_ip + ":" +
                             std::to_string(destPort));
        return ConnectionResult(true, destIP, destPort, sequence_num_second + 1,
                                seq_num_third + 1);
      }
    } catch (const std::exception &e) {
      commandLine(
          '!', "[ERROR] [" +
                   status_strings[static_cast<int>(connection->getStatus())] +
                   "] " + std::string(e.what()));
    }
  }

  commandLine(
      '!',
      "[ERROR] [" + status_strings[static_cast<int>(connection->getStatus())] +
          "] Handshake failed after " + std::to_string(retries) + " retries");
  return ConnectionResult(false, dest_ip, dest_port, 0, 0);
}

ConnectionResult Server::listenBroadcast() {
  for (int i = 0; i < SERVER_MAX_TRY; i++) {
    try {
      Message answer =
          connection->consumeBuffer("", 0, 0, 0, 0, SERVER_BROADCAST_TIMEOUT);
      connection->setStatus(TCPStatusEnum::LISTENING);
      std::cout << answer.ip << std::endl;
      commandLine('+', "Received Broadcast Message\n");
      commandLine('+', "Received Broadcast Message");
      Segment temp = accBroad();
      updateChecksum(temp);
      connection->sendSegment(temp, answer.ip, answer.port);
      return ConnectionResult(true, answer.ip, answer.port,
                              answer.segment.seqNum, answer.segment.ackNum);
    } catch (const std::runtime_error &e) {
      commandLine('x', "Timeout " + std::to_string(i + 1));
      continue;
    }
  }
  return ConnectionResult(false, 0, 0, 0, 0);
}

ConnectionResult Server::startFin(string dest_ip, uint16_t dest_port,
                                  uint32_t seqNum, uint32_t ackNum) {
  uint32_t finSeqNum;
  for (int i = 0; i < SERVER_MAX_TRY; i++) {
    try {
      // Send Fin
      Segment finSeg = fin(seqNum + 1, ackNum);
      updateChecksum(finSeg);
      connection->sendSegment(finSeg, dest_ip, dest_port);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(finSeg.seqNum) + "] [A=" +
                   to_string(finSeg.ackNum) + "] Sending FIN request to " +
                   dest_ip + ":" + to_string(dest_port));
      finSeqNum = finSeg.seqNum;
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
      break;
    } catch (const std::runtime_error &e) {
      commandLine('x', "Timeout " + std::to_string(i + 1));
      continue;
    }
  }

  for (int i = 0; i < SERVER_MAX_TRY; i++) {
    try {
      // REC FIN
      Message fin2 =
          connection->consumeBuffer(dest_ip, dest_port, 0, finSeqNum + 1,
                                    FIN_FLAG, SERVER_COMMON_TIMEOUT);
      commandLine(
          '+', "[" + status_strings[static_cast<int>(connection->getStatus())] +
                   "] [S=" + to_string(fin2.segment.seqNum) +
                   "] [A=" + to_string(fin2.segment.ackNum) +
                   "] Received FIN request from  " + dest_ip +
                   to_string(dest_port));

      // Send ACK
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
    } catch (const std::runtime_error &e) {
      commandLine('x', "Timeout " + std::to_string(i + 1));
      continue;
    }
  }
  return ConnectionResult(false, dest_ip, dest_port, 0, 0);
}

void Server::run() {
  connection->listen();
  connection->startListening();
  while (true) {
    ConnectionResult statusBroadcast = listenBroadcast();
    if (statusBroadcast.success) {
      ConnectionResult statusHandshake =
          respondHandshake(statusBroadcast.ip, statusBroadcast.port);
      if (statusHandshake.success) {
        connection->setStatus(TCPStatusEnum::ESTABLISHED);
        bool isFile = true;
        string fileFullName;
        if (fileEx == "-1") {
          isFile = false;
        } else {
          fileFullName = fileEx == "" ? fileName : fileName + "." + fileEx;
        }

        ConnectionResult statusSend = connection->sendBackN(
            (uint8_t *)item.data(), static_cast<uint32_t>(item.length()),
            statusBroadcast.ip, statusBroadcast.port, statusHandshake.ackNum,
            isFile, fileFullName);
        if (statusSend.success) {
          connection->setStatus(TCPStatusEnum::CLOSING);
          ConnectionResult statusFin =
              startFin(statusBroadcast.ip, statusBroadcast.port,
                       statusHandshake.seqNum, statusHandshake.ackNum);
        }
      }
    }
  }
}
