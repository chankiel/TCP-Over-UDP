#include "socket.hpp"
#include "segment.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

TCPSocket::TCPSocket(string ip, int port) {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    throw std::runtime_error("Failed to create socket");
  }
  this->ip = ip;
  this->port = port;
}

void TCPSocket::listen(string ip, int port) {
  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
  serverAddr.sin_port = htons(port);

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
  if (ip == "0.0.0.0") {
    // std::cout << "ini broadcast" << std::endl;
    clientAddr.sin_addr.s_addr = INADDR_BROADCAST;
  } else {
    clientAddr.sin_addr.s_addr = inet_addr(ip.c_str());
  }
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
  int broadcast = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
                 sizeof(broadcast)) < 0) {
    cout << "Error in setting Broadcast option";
    // close(sockfd);
    return 0;
  }
  sockaddr_in clientAddr;
  // sockaddr_in serverAddr;

  clientAddr.sin_family = AF_INET;
  clientAddr.sin_port = htons(port);
  clientAddr.sin_addr.s_addr = INADDR_ANY;
  // clientAddr.sin_addr.s_addr = inet_addr(ip.c_str());

  socklen_t addrLen = sizeof(clientAddr);

  int bytesRead = recvfrom(sockfd, buffer, length, 0,
                           (struct sockaddr *)&clientAddr, &addrLen);
  // }
  // std::cout << "flgs.syn: " << static_cast<Segment *>(buffer)->flags.syn
  //           << std::endl;
  char address[INET_ADDRSTRLEN];
  // getsockname(sockfd, (struct sockaddr *)&clientAddr, &addrLen);
  inet_ntop(AF_INET, &clientAddr.sin_addr, address, sizeof(address));
  ip = address;
  port = ntohs(clientAddr.sin_port);
  if (bytesRead < 0) {
    throw std::runtime_error("Failed to receive data");
  }

  char clientIp[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
  std::cout << "Received data from " << clientIp << ":"
            << ntohs(clientAddr.sin_port) << std::endl;

  return bytesRead;
}

string TCPSocket::getIP() { return this->ip; }
void TCPSocket::close() {
  if (sockfd >= 0) {
    ::close(sockfd);
    sockfd = -1;
    std::cout << "Socket closed" << std::endl;
  }
}
int32_t TCPSocket::getPort() { return this->port; }

// TCPSocket::~TCPSocket() { close(); }
