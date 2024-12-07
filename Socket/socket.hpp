#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "../Message/message.hpp"
#include "../Segment/segment.hpp"
#include "../Segment/segment_handler.hpp"
#include "../Socket/connection_result.hpp"
#include <arpa/inet.h>
#include <condition_variable>
#include <cstring>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include "../tools/tools.hpp"

using std::condition_variable;
using std::lock_guard;
using std::mutex;
using std::string;
using std::vector;

constexpr uint32_t DEFAULT_TIMEOUT = 2;

enum class TCPStatusEnum
{
  LISTENING,
  SYN_SENT,
  SYN_RECEIVED,
  ESTABLISHED,
  FIN_WAIT_1,
  FIN_WAIT_2,
  CLOSE_WAIT,
  CLOSING,
  LAST_ACK,
  TIME_WAIT,
  CLOSED
};

const std::vector<std::string> status_strings = {
    "LISTENING", "SYN_SENT", "SYN_RECEIVED", "ESTABLISHED", "FIN_WAIT_1",
    "FIN_WAIT_2", "CLOSE_WAIT", "CLOSING", "LAST_ACK", "TIME_WAIT", "CLOSED"};

class TCPSocket
{
private:
  string ip;
  int32_t port;
  int32_t sockfd;
  vector<Message> packetBuffer;
  mutex bufferMutex;
  condition_variable bufferCondition;
  TCPStatusEnum status;
  bool isListening;
  std::thread listenerThread;
  SegmentHandler *sh;

  sockaddr_in createSockAddr(const string &ipAddress, int port);

public:
  explicit TCPSocket(const string &ip, int port);
  ~TCPSocket();
  void bindSocket();
  void setBroadcast();
  void listen();

  void startListening();
  void stopListening();

  bool send(const string &destinationIP, int32_t destinationPort, void *data,
            uint32_t size);
  void sendSegment(const Segment &segment, const string &destinationIP,
                   uint16_t destinationPort);

  int32_t receive(void *buffer, uint32_t bufferSize, bool peek = false);

  void produceBuffer();
  Message consumeBuffer(const string &filterIP = "", uint16_t filterPort = 0,
                        uint32_t filterSeqNum = 0, uint32_t filterAckNum = 0,
                        uint8_t filterFlags = 0, int timeout = 10);

  void sendBackN(uint8_t *dataStream, uint32_t dataSize,
                 const string &destIP, uint16_t destPort, uint32_t startingSeqNum);
  void receiveBackN(vector<Segment> &resBuffer,string dest_ip, uint16_t dest_port, uint32_t seqNum);

  void setStatus(TCPStatusEnum newState);
  TCPStatusEnum getStatus() const;
  void close();
};

#endif