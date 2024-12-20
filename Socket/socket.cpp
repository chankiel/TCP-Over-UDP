#include "socket.hpp"
#include <chrono>
#include <iostream>
#include <iterator>
#include <sys/types.h>

int BROADCAST_TIMEOUT = 12;
int COMMON_TIMEOUT = 12;
int MAX_TRY = 10;

TCPSocket::TCPSocket(const string &ip, int port)
    : ip(ip), port(port), isListening(false)
{
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
  {
    throw std::runtime_error("Socket creation failed.");
  }
}

TCPSocket::~TCPSocket()
{
  stopListening();
  close();
}

std::string TCPSocket::getClientKey(const string &ip, int port){
  return ip+":"+std::to_string(port);
}

void TCPSocket::setStatusConnection(TCPStatusEnum newState,string &ip, int port) { 
  lock_guard<mutex> lock(tableMutex);
  std::string clientKey = getClientKey(ip,port);
  connectionTable[clientKey].status = newState;
}

std::string TCPSocket::getStatusConnection(const string &ip, int port)  {
  lock_guard<mutex> lock(tableMutex);
  std::string clientKey = getClientKey(ip,port);
  return status_strings[static_cast<int>(connectionTable[clientKey].status)];
}

void TCPSocket::setStatus(TCPStatusEnum newState){
  status = newState;
}

TCPStatusEnum TCPSocket::getStatus(){
  return status;
}

void TCPSocket::addNewConnection(const std::string &ip, int port){
  lock_guard<mutex> lock(tableMutex);
  std::string clientKey = getClientKey(ip,port);
  connectionTable[clientKey] = ClientConnection();
}

void TCPSocket::deleteNewConnection(const std::string &ip, int port){
  lock_guard<mutex> lock(tableMutex);
  std::string clientKey = getClientKey(ip,port);
  connectionTable.erase(clientKey);
}

void TCPSocket::listen()
{
  sockaddr_in sockAddr = createSockAddr(ip, port);

  if (bind(sockfd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) < 0)
  {
    throw std::runtime_error("Failed to bind socket");
  }

  std::cout << OUT << " Socket bound and ready to receive data on port " << port
            << std::endl;
}

void TCPSocket::bindSocket()
{
  struct sockaddr_in sockAddr = createSockAddr(ip, port);
  if (bind(sockfd, (const struct sockaddr *)&sockAddr, sizeof(sockAddr)) < 0)
  {
    exit(EXIT_FAILURE);
  }
}

void TCPSocket::setBroadcast()
{
  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) <
      0)
  {
    std::cout << "Broadcast failed" << std::endl;
    ::close(sockfd);
    exit(EXIT_FAILURE);
  }
}

sockaddr_in TCPSocket::createSockAddr(const string &ipAddress, int port)
{
  sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  if (inet_pton(AF_INET, ipAddress.c_str(), &address.sin_addr) <= 0)
  {
    throw std::runtime_error("Invalid IP address format.");
  }
  return address;
}

bool TCPSocket::send(const string &destinationIP, int32_t destinationPort,
                     void *data, uint32_t size)
{
  auto destAddress = createSockAddr(destinationIP, destinationPort);
  if (sendto(sockfd, data, size, 0, (struct sockaddr *)&destAddress,
             sizeof(destAddress)) < 0)
  {
    return false;
  }
  return true;
}

void TCPSocket::sendSegment(const Segment &segment, const string &destinationIP,
                            uint16_t destinationPort)
{
  uint32_t segmentSize = segment.payloadSize + 24;
  auto *buffer = new uint8_t[segmentSize];
  encodeSegment(segment, buffer);
  send(destinationIP, destinationPort, buffer, segmentSize);
  delete[] buffer;
}

void TCPSocket::produceBuffer()
{
  while (isListening)
  {
    try
    {
      uint8_t *dataBuffer = new uint8_t[MAX_SEGMENT_SIZE];
      sockaddr_in clientAddress;
      socklen_t addressLength = sizeof(clientAddress);

      int bytesRead =
          recvfrom(sockfd, dataBuffer, MAX_SEGMENT_SIZE, 0,
                   (struct sockaddr *)&clientAddress, &addressLength);
      if (bytesRead <= 0)
      {
        delete[] dataBuffer;
        if (!isListening)
          break;
        continue;
      }

      Segment segment = decodeSegment(dataBuffer, bytesRead);
      delete[] dataBuffer;

      if (!isValidChecksum(segment))
      {
        continue;
      }

      // if (!isValidCRC(segment))
      // {
      //   std::cout<<"Invalid CRC"<<std::endl;
      //   continue;
      // }

      Message message(inet_ntoa(clientAddress.sin_addr),
                      ntohs(clientAddress.sin_port), segment);

      {
        lock_guard<mutex> lock(bufferMutex);
        packetBuffer.push_back(std::move(message));
        bufferCondition.notify_one();
      }
    }
    catch (const std::exception &ex)
    {
      if (isListening)
      {
        std::cerr << "Error in producer: " << ex.what() << "\n";
      }
    }
  }
}

Message TCPSocket::consumeBuffer(const string &filterIP, uint16_t filterPort,
                                 uint32_t filterSeqNum, uint32_t filterAckNum,
                                 uint8_t filterFlags, int timeout)
{
  auto start = std::chrono::steady_clock::now();
  auto timeoutPoint = (timeout > 0)
                          ? start + std::chrono::seconds(timeout)
                          : std::chrono::steady_clock::time_point::max();
  while (isListening)
  {
    std::unique_lock<mutex> lock(bufferMutex);
    bufferCondition.wait_for(lock, std::chrono::milliseconds(100),
                             [this]()
                             { return !packetBuffer.empty(); });
    for (auto it = packetBuffer.begin(); it != packetBuffer.end(); ++it)
    {
      const auto &msg = *it;
      if ((filterIP.empty() || msg.ip == filterIP) &&
          (filterPort == 0 || msg.port == filterPort) &&
          (filterSeqNum == 0 || msg.segment.seqNum == filterSeqNum) &&
          (filterAckNum == 0 || msg.segment.ackNum == filterAckNum) &&
          (filterFlags == 0 || getFlags8(&msg.segment) == filterFlags))
      {
        Message result = std::move(*it);
        packetBuffer.erase(it);
        return result;
      }
    }
    if (timeout > 0 && std::chrono::steady_clock::now() > timeoutPoint)
    {
      throw std::runtime_error("Buffer consumer timeout.");
    }
  }

  throw std::runtime_error("Socket is no longer listening.");
}

void TCPSocket::startListening()
{
  isListening = true;
  listenerThread = std::thread(&TCPSocket::produceBuffer, this);
}

void TCPSocket::stopListening()
{
  isListening = false;
  if (listenerThread.joinable())
  {
    listenerThread.join();
  }
}

void TCPSocket::close()
{
  if (sockfd >= 0)
  {
    ::close(sockfd);
    sockfd = -1;
    std::cout << "Socket closed" << std::endl;
  }
}

string TCPSocket::concatenatePayloads(vector<Segment> &segments)
{
  string concatenatedData;
  for (const auto &segment : segments)
  {
    if (segment.payload != nullptr && segment.payloadSize > 0)
    {
      concatenatedData.append(reinterpret_cast<char *>(segment.payload),
                              segment.payloadSize);
    }
  }
  return concatenatedData;
}

ConnectionResult TCPSocket::findBroadcast(string dest_ip, uint16_t dest_port)
{
  setBroadcast();
  for (int i = 0; i < MAX_TRY; i++)
  {
    try
    {
      Segment temp = broad();
      sendSegment(temp, dest_ip, dest_port);
      commandLine('i', "Sending Broadcast");
      Message answer =
          consumeBuffer("", 0, 0, 0, 255, BROADCAST_TIMEOUT);
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

ConnectionResult TCPSocket::startHandshake(string dest_ip, uint16_t dest_port)
{
  uint32_t r_seq_num = generateRandomNumber(10, 4294967295);

  commandLine('i', "Sender Program's Three Way Handshake");

  Segment synSegment = syn(r_seq_num);

  for (int i = 0; i < 10; i++)
  {
    try
    {
      sendSegment(synSegment, dest_ip, dest_port);
      setStatus(TCPStatusEnum::SYN_SENT);

      commandLine(
          'i', "[" + status_strings[static_cast<int>(getStatus())] +
                   "] [S=" + std::to_string(r_seq_num) +
                   "] Sending SYN request to " + dest_ip + ":" +
                   std::to_string(dest_port));

      Message result = consumeBuffer(
          dest_ip, dest_port, 0, r_seq_num + 1, SYN_ACK_FLAG, 10);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(getStatus())] +
                   "] [S=" + std::to_string(result.segment.seqNum) +
                   "] [A=" + std::to_string(result.segment.ackNum) +
                   "] Received SYN-ACK request to " + dest_ip + ":" +
                   std::to_string(dest_port));

      uint32_t ackNum = result.segment.seqNum + 1;
      Segment ackSegment = ack(r_seq_num + 1, ackNum);

      sendSegment(ackSegment, dest_ip, dest_port);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(getStatus())] +
                   "] [S=" + std::to_string(ackSegment.seqNum) +
                   "] [A=" + std::to_string(ackSegment.ackNum) +
                   "] Sending ACK request to " + dest_ip + ":" +
                   std::to_string(dest_port));
      commandLine('~', "Ready to receive input from " + dest_ip + ":" +
                           std::to_string(dest_port));
      setStatus(TCPStatusEnum::ESTABLISHED);
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

ConnectionResult TCPSocket::respondFin(string dest_ip, uint16_t dest_port,
                                    uint32_t seqNum, uint32_t ackNum, uint32_t recfin_seqnum)
{
  bool isFirst = true;
  uint32_t recfin_seq = recfin_seqnum;
  for (int i = 0; i < MAX_TRY; i++)
  {
    try
    {
      setStatus(TCPStatusEnum::FIN_WAIT_1);
      if(!isFirst){
        Message rec_fin = consumeBuffer(
            dest_ip, dest_port, 0, seqNum + 1, FIN_FLAG, COMMON_TIMEOUT);
        recfin_seq = rec_fin.segment.seqNum;
      }

      commandLine(
          '+', "[" + status_strings[static_cast<int>(getStatus())] +
                   "] [S=" + to_string(recfin_seq) +
                   "] [A=" + to_string(seqNum+1) +
                   "] Received FIN request from  " + dest_ip +
                   to_string(dest_port));
      // Send ACK
      setStatus(TCPStatusEnum::FIN_WAIT_2);
      Segment ackSeg = ack(seqNum + 1, recfin_seq + 1);
      sendSegment(ackSeg, dest_ip, dest_port);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(getStatus())] +
                   "] [S=" + to_string(ackSeg.seqNum) + "] [A=" +
                   to_string(ackSeg.ackNum) + "] Send ACK request from  " +
                   dest_ip + to_string(dest_port));
      // Send FIN
      setStatus(TCPStatusEnum::TIME_WAIT);
      Segment finSeg = fin(seqNum + 2, recfin_seq + 1);
      sendSegment(finSeg, dest_ip, dest_port);
      commandLine(
          'i', "[" + status_strings[static_cast<int>(getStatus())] +
                   "] [S=" + to_string(finSeg.seqNum) + "] [A=" +
                   to_string(finSeg.ackNum) + "] Send FIN request from  " +
                   dest_ip + to_string(dest_port));

      // REC ACK
      setStatus(TCPStatusEnum::CLOSED);
      Message answer_fin =
          consumeBuffer(dest_ip, dest_port, 0, finSeg.seqNum + 1,
                                    ACK_FLAG, COMMON_TIMEOUT);
      commandLine(
          '+', "[" + status_strings[static_cast<int>(getStatus())] +
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

// Server Role
ConnectionResult TCPSocket::respondHandshake(string dest_ip, uint16_t dest_port)
{
  for (int i = 0; i < MAX_TRY; i++)
  {
    try
    {
      Message sync_message =
          consumeBuffer(dest_ip, dest_port, 0, 0, SYN_FLAG, 10);
      setStatusConnection(TCPStatusEnum::SYN_RECEIVED,dest_ip,dest_port);
      std::string destIP = sync_message.ip;
      uint16_t destPort = sync_message.port;
      uint32_t sequence_num_first = sync_message.segment.seqNum;

      commandLine(
          'i', "[" + getStatusConnection(dest_ip,dest_port) +
                   "] [S=" + std::to_string(sequence_num_first) +
                   "] Received SYN request from " + dest_ip + ":" +
                   std::to_string(destPort));

      // Sending SYN-ACK Request
      uint32_t sequence_num_second = generateRandomNumber(1, 1000);
      uint32_t ack_num_second = sequence_num_first + 1;

      commandLine(
          'i', "[" + getStatusConnection(dest_ip, dest_port) +
                   "] [S=" + std::to_string(sequence_num_second) +
                   "] [A=" + std::to_string(ack_num_second) +
                   "] Sending SYN-ACK request to " + dest_ip + ":" +
                   std::to_string(destPort));
      Segment synSeg = synAck(sequence_num_second, ack_num_second);
      sendSegment(synSeg, dest_ip, dest_port);
      setStatusConnection(TCPStatusEnum::SYN_SENT,dest_ip,dest_port);

      // Received ACK Request
      Message ack_message =
          consumeBuffer(destIP, destPort, 0, 0, ACK_FLAG);
      setStatusConnection(TCPStatusEnum::ESTABLISHED,dest_ip,dest_port);
      uint32_t ack_num_third = ack_message.segment.ackNum;
      uint32_t seq_num_third = ack_message.segment.seqNum;
      commandLine(
          'i', "[" + getStatusConnection(dest_ip,dest_port) +
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
      cout << ERROR << brackets("TIMEOUT") + "Restarting Handshake" + brackets("ATTEMPT-" + std::to_string(i + 1)) << std::endl;
    }
  }
  throw std::runtime_error("Failed to receive broadcast. Restarting listening for Broadcast.");
}

ConnectionResult TCPSocket::listenBroadcast()
{
  std::cout << std::endl;
  commandLine('i', "Listening to the broadcast port for clients.");
  try
  {
    Message answer =
        consumeBuffer("",0, 0, 0, 128, BROADCAST_TIMEOUT);
    commandLine('+', "Received Broadcast Message");
    Segment temp = accBroad();
    sendSegment(temp,answer.ip,answer.port);
    return ConnectionResult(true, answer.ip, answer.port,
                            answer.segment.seqNum, answer.segment.ackNum);
  }
  catch (const std::runtime_error &e)
  {
    throw std::runtime_error("Didn't found any Broadcast Request. Continue listening");
  }
}

ConnectionResult TCPSocket::startFin(string dest_ip, uint16_t dest_port,
                                  uint32_t seqNum, uint32_t ackNum)
{
  for (int i = 0; i < MAX_TRY; i++)
  {
    try
    {
      setStatusConnection(TCPStatusEnum::CLOSE_WAIT,dest_ip,dest_port);
      Segment finSeg = fin(seqNum + 1, ackNum);
      sendSegment(finSeg, dest_ip, dest_port);
      commandLine(
          'i', "[" + getStatusConnection(dest_ip,dest_port) +
                   "] [S=" + to_string(finSeg.seqNum) + "] [A=" +
                   to_string(finSeg.ackNum) + "] Sending FIN request to " +
                   dest_ip + ":" + to_string(dest_port));
      // REC ACK
      Message answer_fin =
          consumeBuffer(dest_ip, dest_port, 0, finSeg.seqNum + 1,
                                    ACK_FLAG, COMMON_TIMEOUT);
      commandLine(
          '+', "[" + getStatusConnection(dest_ip,dest_port) +
                   "] [S=" + to_string(answer_fin.segment.seqNum) +
                   "] [A=" + to_string(answer_fin.segment.ackNum) +
                   "] Received ACK request from  " + dest_ip +
                   to_string(dest_port));
      // REC FIN
      setStatus(TCPStatusEnum::LAST_ACK);
      Message fin2 =
          consumeBuffer(dest_ip, dest_port, 0, finSeg.seqNum + 1,
                                    FIN_FLAG, COMMON_TIMEOUT);
      commandLine(
          '+', "[" + status_strings[static_cast<int>(getStatus())] +
                   "] [S=" + to_string(fin2.segment.seqNum) +
                   "] [A=" + to_string(fin2.segment.ackNum) +
                   "] Received FIN request from  " + dest_ip +
                   to_string(dest_port));

      // Send ACK
      setStatus(TCPStatusEnum::CLOSED);
      Segment ackSeg = ack(seqNum + 2, fin2.segment.seqNum + 1);
      sendSegment(ackSeg, dest_ip, dest_port);

      commandLine(
          'i', "[" + status_strings[static_cast<int>(getStatus())] +
                   "] [S=" + to_string(ackSeg.seqNum) + "] [A=" +
                   to_string(ackSeg.ackNum) + "] Sending FIN request to " +
                   dest_ip + ":" + to_string(dest_port));

      commandLine('i', "Connection Closed");
      return ConnectionResult(true, dest_ip, dest_port, 0, 0);
    }
    catch (const std::runtime_error &e)
    {
      cout << ERROR << brackets("TIMEOUT") + "No respond for FIN request. Restarting sending for FIN to Client" + brackets("ATTEMPT-" + std::to_string(i + 1)) << std::endl;
      continue;
    }
  }
  throw std::runtime_error("Handshake response failed. Restarting listening for Broadcast.");
}

ConnectionResult TCPSocket::sendBackN(uint8_t *dataStream, uint32_t dataSize,
                                      const string &destIP, uint16_t destPort,
                                      uint32_t startingSeqNum, bool isFile,
                                      string fileFullName)
{
  SegmentHandler *sh = nullptr;
  std::string clientKey = getClientKey(destIP,destPort);
  {
      lock_guard<mutex> lock(tableMutex);
      sh = connectionTable[clientKey].sh.get();
  }
  sh->setDataStream(dataStream, dataSize, startingSeqNum, port, destPort);
  if (isFile)
  {
    sh->addMetadata(fileFullName, port, destPort);
  }
  sh->markEOF();

  vector<thread> threads;
  std::atomic<bool> retry(false);
  while (true)
  {
    while (sh->getCurrentSeqNum() - sh->getCurrentAckNum() <
           sh->getWindowSize())
    {
      Segment *seg = sh->advanceWindow(1);
      if (seg == nullptr)
      {
        break;
      }
      threads.emplace_back([this,sh, seg = *seg, destIP, destPort, startingSeqNum,
                            &retry]()
                           {
        try {
          std::cout << OUT << brackets(getStatusConnection(destIP,destPort))
                    << brackets("Seq " +
                                std::to_string(seg.seqNum - startingSeqNum))
                    << brackets("S=" + std::to_string(seg.seqNum)) << "Sent"
                    << endl;
          sendSegment(seg, destIP, destPort);

          Message result =
              consumeBuffer(destIP, destPort, 0, seg.seqNum + 1, ACK_FLAG, 1);

          std::cout << IN
                    << brackets(getStatusConnection(destIP,destPort))<<brackets("A=" +
                           std::to_string(result.segment.ackNum))+
                           "Received ACK request from " + result.ip + ":" +
                           std::to_string(result.port)
                    << std::endl;
          ;
          sh->ackWindow(seg.seqNum);
        } catch (const std::runtime_error &e) {
          std::cout << OUT << brackets("TIMEOUT")
                    << brackets("Seq " +
                                std::to_string(seg.seqNum - startingSeqNum))
                    << brackets("S=" + std::to_string(seg.seqNum)) << "Timeout"
                    << endl;
          if (sh->getCurrentAckNum() + 1 == seg.seqNum) {
            retry = true;
            sh->goBackWindow();
          }
        } });
    }
    if (sh->isFinished(startingSeqNum))
    {
      break;
    }

    if (retry)
    {
      for (auto &t : threads)
      {
        if (t.joinable())
        {
          t.join();
        }
      }
      threads.clear();
      retry = false;
    }
  }
  for (auto &t : threads)
  {
    if (t.joinable())
    {
      t.join();
    }
  }
  threads.clear();

  std::cout << OUT << brackets(status_strings[(int)status])
            << "All segments sent to " << destIP << ":" << destPort << endl;
  return ConnectionResult(true, destIP, destPort, 0, 0);
}

ConnectionResult TCPSocket::receiveBackN(vector<Segment> &resBuffer,
                                         string destIP, uint16_t destPort,
                                         uint32_t seqNum) {
  int i = 0;
  bool finished = false;
  int limit = 0;
  uint32_t seqNumIt = seqNum;
  while (limit < 10 && !finished) {
    try {
      Message res = consumeBuffer(destIP, destPort);
      if (res.segment.flags.fin == 1) {
        return ConnectionResult(true, destIP, destPort, res.segment.seqNum, res.segment.ackNum);
      }

      if (res.segment.seqNum < seqNumIt) {
        Segment temp = ack(0, res.segment.seqNum + 1);
        sendSegment(temp, destIP, destPort);
      }
      if (res.segment.seqNum == seqNumIt) {
        i++;
        resBuffer.push_back(res.segment);
        std::cout << IN << brackets(status_strings[(int)status])
                  << brackets("Seq " + std::to_string(i))
                  << brackets("S=" + std::to_string(res.segment.seqNum))
                  << "ACKed" << endl;
        seqNumIt++;
        Segment temp = ack(0, seqNumIt);
        sendSegment(temp, destIP, destPort);

        std::cout << OUT << brackets(status_strings[(int)status])
                  << brackets("Seq " + std::to_string(i))
                  << brackets("A=" + std::to_string(seqNumIt)) << "Sent"
                  << endl;
      }
    } catch (const std::exception &e) {
      limit++;
      commandLine('!', "[ERROR] " + brackets(status_strings[(int)status]) + std::string(e.what()));
    }
  }
  throw std::runtime_error("Receiving data Process Failed. Terminating Client. Thank you!");
}