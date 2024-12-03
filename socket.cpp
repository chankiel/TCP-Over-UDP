#include "socket.hpp"
#include "segment.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

TCPSocket::TCPSocket(int port_) {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  port = port_;
  if (sockfd < 0) {
    throw std::runtime_error("Failed to create socket");
  }
  sockaddr_in serverAddr;
}

void TCPSocket::listen() {
  getStatus();
  // std::cout << "testing: " << std::endl;
  // Segment synSeg = syn(1);
  // Segment ackSeg = ack(2, 3);
  // std::cout << "synSeg: " << synSeg.flags.ack << std::endl;
  // std::cout << "ackSeg: " << ackSeg.flags.ack << std::endl;
  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);
  if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    throw std::runtime_error("Failed to bind socket");
  }

  std::cout << "Socket bound and ready to receive data on port " << port
            << std::endl;
  status = LISTEN;
}

void TCPSocket::send(std::string ip, int32_t port, void *dataStream,
                     uint32_t dataSize, int isClient) {
  getStatus();
  sockaddr_in servAddr, cliAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(port);
  if (isClient) {
    std::cout << "sent to addr: " << &servAddr << std::endl;
    Segment *tempSeg = static_cast<Segment *>(dataStream);
    std::cout << "flags.syn: " << tempSeg->flags.syn << std::endl;
    std::cout << "flags.ack: " << tempSeg->flags.ack << std::endl;
    if (inet_pton(AF_INET, ip.c_str(), &servAddr.sin_addr) <= 0) {
      throw std::runtime_error("Invalid address or address not supported");
    }

    if (sendto(sockfd, dataStream, dataSize, 0, (struct sockaddr *)&servAddr,
               sizeof(servAddr)) < 0) {
      throw std::runtime_error("Failed to send data");
    }

    std::cout << "Data sent to " << ip << ":" << port << std::endl;
    if (tempSeg->flags.syn == 1 && tempSeg->flags.ack == 0) {
      status = SYN_SENT;
    }
  } else {
    std::cout << "sent to addr: " << &cliAddr << std::endl;
    Segment *tempSeg = static_cast<Segment *>(dataStream);
    std::cout << "flags.syn: " << tempSeg->flags.syn << std::endl;
    std::cout << "flags.ack: " << tempSeg->flags.ack << std::endl;
    if (inet_pton(AF_INET, ip.c_str(), &cliAddr.sin_addr) <= 0) {
      throw std::runtime_error("Invalid address or address not supported");
    }

    if (sendto(sockfd, dataStream, dataSize, 0, (struct sockaddr *)&cliAddr,
               sizeof(cliAddr)) < 0) {
      throw std::runtime_error("Failed to send data");
    }

    std::cout << "Data sent to " << ip << ":" << port << std::endl;
    if (tempSeg->flags.syn == 1 && tempSeg->flags.ack == 0) {
      status = SYN_SENT;
    }
  }
}

int32_t TCPSocket::ambil(void *buffer, uint32_t length, int isClient) {
  getStatus();
  sockaddr_in servAddr, cliAddr;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(port);
  if (isClient) {
    std::cout << "isClient: " << isClient << std::endl;
    std::cout << "recv fom addr: " << &servAddr << std::endl;
    socklen_t addrLen = sizeof(servAddr);
    std::cout << "sblm recvFrom: " << std::endl;
    // int bytesRead = recv(sockfd, buffer, length, 0);
    int bytesRead = recvfrom(sockfd, buffer, length, 0,
                             (struct sockaddr *)&servAddr, &addrLen);
    std::cout << bytesRead << std::endl;
    std::cout << "sesudah recvFrom: " << std::endl;
    if (bytesRead < 0) {
      throw std::runtime_error("Failed to receive data");
    }
    Segment *tempSeg = static_cast<Segment *>(buffer);
    std::cout << "buffer: " << tempSeg->flags.syn << std::endl;
    if (tempSeg->flags.syn == 1 && tempSeg->flags.ack == 0) {
      status = SYN_RECEIVED;
    }
    // if (tempSeg->flags.syn == 1 && tempSeg->flags.ack == 1) {
    //   // status = SYN_RECEIVED;
    //   std::cout << "udah terima syn-ack" << std::endl;
    // }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &servAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
    std::cout << "Received data from " << clientIp << ":"
              << ntohs(servAddr.sin_port) << std::endl;

    return bytesRead;
  } else {
    std::cout << "isClient: " << isClient << std::endl;
    std::cout << "recv fom addr: " << &cliAddr << std::endl;
    socklen_t addrLen = sizeof(cliAddr);
    std::cout << "sblm recvFrom: " << std::endl;
    // int bytesRead = recv(sockfd, buffer, length, 0);
    int bytesRead = recvfrom(sockfd, buffer, length, 0,
                             (struct sockaddr *)&cliAddr, &addrLen);
    std::cout << bytesRead << std::endl;
    std::cout << "sesudah recvFrom: " << std::endl;
    if (bytesRead < 0) {
      throw std::runtime_error("Failed to receive data");
    }
    Segment *tempSeg = static_cast<Segment *>(buffer);
    std::cout << "buffer: " << tempSeg->flags.syn << std::endl;
    if (tempSeg->flags.syn == 1 && tempSeg->flags.ack == 0) {
      status = SYN_RECEIVED;
    }
    // if (tempSeg->flags.syn == 1 && tempSeg->flags.ack == 1) {
    //   // status = SYN_RECEIVED;
    //   std::cout << "udah terima syn-ack" << std::endl;
    // }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cliAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
    std::cout << "Received data from " << clientIp << ":"
              << ntohs(cliAddr.sin_port) << std::endl;

    return bytesRead;
  }
}

void TCPSocket::close() {
  getStatus();
  if (sockfd >= 0) {
    ::close(sockfd);
    sockfd = -1;
    std::cout << "Socket closed" << std::endl;
  }
}

void TCPSocket::getStatus() { std::cout << "Status: " << status << std::endl; }

// TCPSocket::~TCPSocket() { close(); }
