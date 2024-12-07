#include "socket.hpp"
#include <chrono>
#include <iostream>
#include <iterator>
#include <sys/types.h>

TCPSocket::TCPSocket(const string &ip, int port)
    : ip(ip), port(port), isListening(false) {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    throw std::runtime_error("Socket creation failed.");
  }
  status = TCPStatusEnum::CLOSED;
  sh = new SegmentHandler();
}

TCPSocket::~TCPSocket() {
  stopListening();
  close();
  delete sh;
}

void TCPSocket::listen() {
  sockaddr_in sockAddr = createSockAddr(ip, port);

  if (bind(sockfd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) < 0) {
    throw std::runtime_error("Failed to bind socket");
  }

  std::cout << "Socket bound and ready to receive data on port " << port
            << std::endl;
}

void TCPSocket::bindSocket() {
  struct sockaddr_in sockAddr = createSockAddr(ip, port);
  if (bind(sockfd, (const struct sockaddr *)&sockAddr, sizeof(sockAddr)) < 0) {
    exit(EXIT_FAILURE);
  }
}

void TCPSocket::setBroadcast() {
  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) <
      0) {
    std::cout << "Broadcast failed" << std::endl;
    ::close(sockfd);
    exit(EXIT_FAILURE);
  }
}

sockaddr_in TCPSocket::createSockAddr(const string &ipAddress, int port) {
  sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  if (inet_pton(AF_INET, ipAddress.c_str(), &address.sin_addr) <= 0) {
    throw std::runtime_error("Invalid IP address format.");
  }
  return address;
}

bool TCPSocket::send(const string &destinationIP, int32_t destinationPort,
                     void *data, uint32_t size) {
  auto destAddress = createSockAddr(destinationIP, destinationPort);
  if (sendto(sockfd, data, size, 0, (struct sockaddr *)&destAddress,
             sizeof(destAddress)) < 0) {
    return false;
  }
  return true;
}

void TCPSocket::sendSegment(const Segment &segment, const string &destinationIP,
                            uint16_t destinationPort) {
  auto updatedSegment = updateChecksum(segment);
  uint32_t segmentSize = updatedSegment.payloadSize + 24;

  auto *buffer = new uint8_t[segmentSize];
  encodeSegment(updatedSegment, buffer);
  send(destinationIP, destinationPort, buffer, segmentSize);
  // cout << "SEND SEGMENT: " << destinationIP << " " << destinationPort << " "
  // << segment.payloadSize << endl;
  delete[] buffer;
}

int32_t TCPSocket::receive(void *buffer, uint32_t bufferSize, bool peek) {
  sockaddr_in sourceAddress = {};
  socklen_t addressLength = sizeof(sourceAddress);

  int flags = peek ? MSG_PEEK : 0;
  return recvfrom(sockfd, buffer, bufferSize, flags,
                  (struct sockaddr *)&sourceAddress, &addressLength);
}

void TCPSocket::produceBuffer() {
  while (isListening) {
    try {
      uint8_t *dataBuffer = new uint8_t[MAX_SEGMENT_SIZE];
      sockaddr_in clientAddress;
      socklen_t addressLength = sizeof(clientAddress);

      int bytesRead =
          recvfrom(sockfd, dataBuffer, MAX_SEGMENT_SIZE, 0,
                   (struct sockaddr *)&clientAddress, &addressLength);
      if (bytesRead <= 0) {
        delete[] dataBuffer;
        if (!isListening)
          break;
        continue;
      }

      Segment segment = decodeSegment(dataBuffer, bytesRead);
      delete[] dataBuffer;
      std::cout << "procedurBuffer debug" << std::endl;
      printSegment(segment);
      std::cout << "procedurBuffer checksum inside: " << segment.checksum
                << std::endl;
      std::cout << "procedurBuffer checksum calc: "
                << calculateChecksum(segment) << std::endl;
      // if (!isValidChecksum(segment)) { // error pas masuk isvalid
      // checksum
      if (segment.checksum != calculateChecksum(segment)) {
        continue;
      }

      Message message(inet_ntoa(clientAddress.sin_addr),
                      ntohs(clientAddress.sin_port), segment);

      {
        lock_guard<mutex> lock(bufferMutex);
        packetBuffer.push_back(std::move(message));
        bufferCondition.notify_one();
      }
    } catch (const std::exception &ex) {
      if (isListening) {
        std::cerr << "Error in producer: " << ex.what() << "\n";
      }
    }
  }
}

Message TCPSocket::consumeBuffer(const string &filterIP, uint16_t filterPort,
                                 uint32_t filterSeqNum, uint32_t filterAckNum,
                                 uint8_t filterFlags, int timeout) {
  auto start = std::chrono::steady_clock::now();
  auto timeoutPoint = (timeout > 0)
                          ? start + std::chrono::seconds(timeout)
                          : std::chrono::steady_clock::time_point::max();
  while (isListening) {
    std::unique_lock<mutex> lock(bufferMutex);
    bufferCondition.wait_for(lock, std::chrono::milliseconds(100),
                             [this]() { return !packetBuffer.empty(); });
    // cout<<packetBuffer.size()<<endl;
    for (auto it = packetBuffer.begin(); it != packetBuffer.end(); ++it) {
      const auto &msg = *it;
      if ((filterIP.empty() || msg.ip == filterIP) &&
          (filterPort == 0 || msg.port == filterPort) &&
          (filterSeqNum == 0 || msg.segment.seqNum == filterSeqNum) &&
          (filterAckNum == 0 || msg.segment.ackNum == filterAckNum) &&
          (filterFlags == 0 || getFlags8(&msg.segment) == filterFlags)) {
        Message result = std::move(*it);
        packetBuffer.erase(it);
        return result;
      }
    }
    if (timeout > 0 && std::chrono::steady_clock::now() > timeoutPoint) {
      throw std::runtime_error("Buffer consumer timeout.");
    }
  }

  throw std::runtime_error("Socket is no longer listening.");
}

void TCPSocket::setStatus(TCPStatusEnum newState) { status = newState; }

TCPStatusEnum TCPSocket::getStatus() const { return status; }

void TCPSocket::startListening() {
  isListening = true;
  listenerThread = std::thread(&TCPSocket::produceBuffer, this);
}

void TCPSocket::stopListening() {
  isListening = false;
  if (listenerThread.joinable()) {
    listenerThread.join();
  }
}

void TCPSocket::close() {
  if (sockfd >= 0) {
    ::close(sockfd);
    sockfd = -1;
    std::cout << "Socket closed" << std::endl;
  }
}

void TCPSocket::sendBackN(uint8_t *dataStream, uint32_t dataSize,
                          const string &destIP, uint16_t destPort,
                          uint32_t startingSeqNum) {
  sh->setDataStream(dataStream, dataSize, startingSeqNum, port, destPort);
  cout << "Awal: " << sh->getCurrentSeqNum() << " " << sh->getCurrentAckNum()
       << endl;
  vector<thread> threads;
  std::atomic<bool> retry(false);
  bool endOfSegBuffer = false;
  while (true) {
    // cout << "SeqNum and AckNum: " << sh->getCurrentSeqNum() << " " <<
    // sh->getCurrentAckNum() << endl;
    while (sh->getCurrentSeqNum() - sh->getCurrentAckNum() <
           sh->getWindowSize()) {
      Segment *seg = sh->advanceWindow(1);
      if (seg == nullptr) {
        // cout << "SeqNum and AckNum: " << sh->getCurrentSeqNum() << " " <<
        // sh->getCurrentAckNum() << endl;

        // cout << "END BUFFER" << endl;
        endOfSegBuffer = true;
        break;
      }
      // cout<<"Luar thread: "<<endl;
      // printSegment(*seg);
      threads.emplace_back([this, seg = *seg, destIP, destPort, startingSeqNum,
                            &retry]() {
        try {
          // cout<<"Dalam thread: "<<endl;
          // printSegment(seg);
          std::cout << OUT << brackets(status_strings[(int)status])
                    << brackets("Seq " +
                                std::to_string(seg.seqNum - startingSeqNum))
                    << brackets("S=" + std::to_string(seg.seqNum)) << "Sent"
                    << endl;
          sendSegment(seg, destIP, destPort);
          Message result =
              consumeBuffer(destIP, destPort, 0, seg.seqNum + 1, ACK_FLAG, 1);
          commandLine(
              'i', "[Established] [A=" + std::to_string(result.segment.ackNum) +
                       "] Received ACK request from " + result.ip + ":" +
                       std::to_string(result.port));
          // printSegment(seg);
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
        }
      });
    }
    if (sh->getCurrentAckNum() == sh->getCurrentSeqNum() && endOfSegBuffer) {
      break;
    }

    if (retry) {
      for (auto &t : threads) {
        if (t.joinable()) {
          t.join();
        }
      }
      threads.clear();
      retry = false;
    }
    // cout << "SeqNum and AckNum: "<<sh->getCurrentSeqNum() << " " <<
    // sh->getCurrentAckNum() << endl;
  }
  for (auto &t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }
  threads.clear();
  std::cout << OUT << brackets(status_strings[(int)status])
            << "All segments sent to " << destIP << ":" << destPort << endl;
}

void TCPSocket::receiveBackN(vector<Segment> &resBuffer, string destIP,
                             uint16_t destPort, uint32_t seqNum) {
  int i = 0;
  bool finished = false;
  int limit = 0;
  uint32_t seqNumIt = seqNum;
  while (limit < 10 && !finished) {
    try {
      Message res = consumeBuffer(destIP, destPort);
      // std::cout << std::to_string(res.segment.seqNum) << " " <<
      // std::to_string(seqNumIt) << endl;
      if (res.segment.seqNum < seqNumIt) {
        // cout << "SIni: " << endl;
        sendSegment(ack(0, res.segment.seqNum + 1), destIP, destPort);
      }

      if (res.segment.seqNum == seqNumIt) {
        i++;
        resBuffer.push_back(res.segment);
        std::cout << IN << brackets(status_strings[(int)status])
                  << brackets("Seq " + std::to_string(i))
                  << brackets("S=" + std::to_string(res.segment.seqNum))
                  << "ACKed" << endl;
        seqNumIt++;
        sendSegment(ack(0, seqNumIt), destIP, destPort);
        std::cout << OUT << brackets(status_strings[(int)status])
                  << brackets("Seq " + std::to_string(i))
                  << brackets("A=" + std::to_string(seqNumIt)) << "Sent"
                  << endl;

        if (res.segment.flags.ece == 1) {
          break;
        }
      }
    } catch (const std::exception &e) {
      limit++;
      commandLine('!', "[ERROR] [Established] " + std::string(e.what()));
    }
  }
}
