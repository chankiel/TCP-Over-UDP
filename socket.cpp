#include "socket.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

TCPSocket::TCPSocket(string ip, int port) {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    throw std::runtime_error("Failed to create socket");
  }
  this->ip = ip;
  this->port = port;
}

void TCPSocket::listen(int servPort) {
  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
  serverAddr.sin_port = htons(servPort);

  if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    throw std::runtime_error("Failed to bind socket");
  }

  std::cout << "Socket bound and ready to receive data on port " << port
            << std::endl;
}

void TCPSocket::send(std::string ip, int32_t port, void *dataStream,
                     uint32_t dataSize) {
  sockaddr_in clientAddr{};
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip.c_str(), &clientAddr.sin_addr) <= 0) {
    throw std::runtime_error("Invalid address or address not supported");
  }

  if (sendto(sockfd, dataStream, dataSize, 0, (struct sockaddr *)&clientAddr,
             sizeof(clientAddr)) < 0) {
    throw std::runtime_error("Failed to send data");
  }

  std::cout << "Data sent to " << ip << ":" << port << std::endl;
}

int32_t TCPSocket::ambil(void *buffer, uint32_t length) {
  sockaddr_in clientAddr{};

  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = inet_addr(ip.c_str());
  clientAddr.sin_port = htons(554);
  socklen_t addrLen = sizeof(clientAddr);

  std::cout << "bfr  recvfrom " << std::endl;
  std::cout << clientAddr.sin_port << std::endl;
  int bytesRead = recvfrom(sockfd, buffer, length, 0,
                           (struct sockaddr *)&clientAddr, &addrLen);
  std::cout << clientAddr.sin_port << std::endl;
  std::cout << "aft  recvfrom " << std::endl;
  if (bytesRead < 0) {
    throw std::runtime_error("Failed to receive data");
  }

  char clientIp[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
  std::cout << "Received data from " << clientIp << ":"
            << ntohs(clientAddr.sin_port) << std::endl;

  return bytesRead;
}

void TCPSocket::close() {
  if (sockfd >= 0) {
    ::close(sockfd);
    sockfd = -1;
    std::cout << "Socket closed" << std::endl;
  }
}

// TCPSocket::~TCPSocket() { close(); }
