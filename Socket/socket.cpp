#include "socket.hpp"
#include <chrono>
#include <iostream>
#include <sys/types.h>

TCPSocket::TCPSocket(const string &ip, int port)
    : ip(ip), port(port), isListening(false) {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    throw std::runtime_error("Socket creation failed.");
  }
  socketState = TCPState::CLOSED;
}

TCPSocket::~TCPSocket() {
  stopListening();
  close();
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

      // if (!isValidChecksum(segment))
      // {
      //     continue;
      // }

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
                                  uint8_t filterFlags, int timeout)
{
    auto start = std::chrono::steady_clock::now();
    auto timeoutPoint = (timeout > 0) ? start + std::chrono::seconds(timeout) : std::chrono::steady_clock::time_point::max();
    while (isListening)
    {
        // std::cout << packetBuffer.size() << std::endl;
        std::unique_lock<mutex> lock(bufferMutex);
        bufferCondition.wait_for(lock, std::chrono::milliseconds(100), [this]() { return !packetBuffer.empty(); });
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
        // std::cout<<timeout<<std::endl;

    if (timeout > 0 && std::chrono::steady_clock::now() > timeoutPoint) {
      throw std::runtime_error("Buffer consumer timeout.");
    }
  }

  throw std::runtime_error("Socket is no longer listening.");
}

void TCPSocket::setSocketState(TCPState newState) { socketState = newState; }

TCPState TCPSocket::getSocketState() const { return socketState; }

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
                          const string &destIP, uint16_t destPort) {
  SegmentHandler segmentHandler;
  segmentHandler.setDataStream(dataStream, dataSize);

  uint32_t base = 0;
  uint32_t nextSeqNum = 0;
  uint32_t windowSize = segmentHandler.getWindowSize();
  uint32_t totalSegments = (dataSize + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE;

  std::map<uint32_t, bool> ackedSegments;
  for (uint32_t i = 0; i < totalSegments; ++i) {
    ackedSegments[i] = false;
  }

  while (base < totalSegments) {
    // Send segments within the window
    while (nextSeqNum < base + windowSize && nextSeqNum < totalSegments) {
      Segment *segment = segmentHandler.advanceWindow(0);
      if (segment) {
        sendSegment(*segment, destIP, destPort);
      }
      nextSeqNum++;
    }

    try {
      // Wait for ACKs
      Message ackMessage =
          consumeBuffer(destIP, destPort, 0, 0, ACK_FLAG, DEFAULT_TIMEOUT);

      uint32_t ackNum = ackMessage.segment.ackNum;
      ackedSegments[ackNum / MAX_PAYLOAD_SIZE] = true;

      // Slide the window
      while (base < totalSegments && ackedSegments[base]) {
        base++;
      }
    } catch (const std::exception &e) {
      // Timeout - retransmit unacked segments
      for (uint32_t i = base; i < nextSeqNum; ++i) {
        if (!ackedSegments[i]) {
          Segment *segment = segmentHandler.advanceWindow(i * MAX_PAYLOAD_SIZE);
          if (segment) {
            sendSegment(*segment, destIP, destPort);
          }
        }
      }
    }
  }
}

void TCPSocket::receieveBackN(string destIP, uint16_t destPort) {
  std::map<uint32_t, Segment> receivedSegments;
  uint32_t expectedSeqNum = 0;
  uint32_t totalDataSize = 0;

  while (true) {
    try {
      Message message = consumeBuffer(destIP, destPort);

      uint32_t seqNum = message.segment.seqNum;
      if (seqNum == expectedSeqNum) {
        receivedSegments[seqNum] = message.segment;
        expectedSeqNum += message.segment.payloadSize;

        // Send ACK for the received segment
        Segment ackSegment = ack(0, expectedSeqNum);
        sendSegment(ackSegment, destIP, destPort);

        // Check if all data has been received
        totalDataSize += message.segment.payloadSize;
        if (message.segment.payloadSize < MAX_PAYLOAD_SIZE) {
          break;
        }
      } else {
        // Out-of-order segment, send ACK for the last received sequence number
        Segment ackSegment = ack(0, expectedSeqNum);
        sendSegment(ackSegment, destIP, destPort);
      }
    } catch (const std::exception &e) {
      std::cerr << "Timeout or error while receiving data: " << e.what()
                << std::endl;
      break;
    }
  }

  // Reassemble the data stream
  for (auto &entry : receivedSegments) {
    // Process the received segments in order
    // Example: Write to a file or buffer
  }
}
