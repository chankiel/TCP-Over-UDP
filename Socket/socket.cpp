#include "socket.hpp"
#include <chrono>
#include <iostream>
#include <iterator>
#include <sys/types.h>

TCPSocket::TCPSocket(const string &ip, int port)
    : ip(ip), port(port), isListening(false)
{
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
  {
    throw std::runtime_error("Socket creation failed.");
  }
  status = TCPStatusEnum::CLOSED;
  sh = new SegmentHandler();
}

TCPSocket::~TCPSocket()
{
  stopListening();
  close();
  delete sh;
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

int32_t TCPSocket::receive(void *buffer, uint32_t bufferSize, bool peek)
{
  sockaddr_in sourceAddress = {};
  socklen_t addressLength = sizeof(sourceAddress);

  int flags = peek ? MSG_PEEK : 0;
  return recvfrom(sockfd, buffer, bufferSize, flags,
                  (struct sockaddr *)&sourceAddress, &addressLength);
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

void TCPSocket::setStatus(TCPStatusEnum newState) { status = newState; }

TCPStatusEnum TCPSocket::getStatus() const { return status; }

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

ConnectionResult TCPSocket::sendBackN(uint8_t *dataStream, uint32_t dataSize,
                                      const string &destIP, uint16_t destPort,
                                      uint32_t startingSeqNum, bool isFile,
                                      string fileFullName)
{
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
      threads.emplace_back([this, seg = *seg, destIP, destPort, startingSeqNum,
                            &retry]()
                           {
        try {
          std::cout << OUT << brackets(status_strings[(int)status])
                    << brackets("Seq " +
                                std::to_string(seg.seqNum - startingSeqNum))
                    << brackets("S=" + std::to_string(seg.seqNum)) << "Sent"
                    << endl;
          sendSegment(seg, destIP, destPort);

          Message result =
              consumeBuffer(destIP, destPort, 0, seg.seqNum + 1, ACK_FLAG, 1);

          std::cout << IN
                    << brackets(status_strings[(int)status])<<brackets("A=" +
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
        updateChecksum(temp);
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
        updateChecksum(temp);
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