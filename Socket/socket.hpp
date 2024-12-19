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
#include <unordered_map>
#include <memory>

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

struct ClientConnection {
  TCPStatusEnum status;
  std::unique_ptr<SegmentHandler> sh;
  ClientConnection(): status(TCPStatusEnum::LISTENING), sh(std::make_unique<SegmentHandler>()){}

  ClientConnection(ClientConnection&&) = default;
  ClientConnection& operator=(ClientConnection&&) = default;

  ClientConnection(const ClientConnection&) = delete;
  ClientConnection& operator=(const ClientConnection&) = delete;
  
};

class TCPSocket
{
private:
  string ip;
  int32_t port;
  int32_t sockfd;
  TCPStatusEnum status;

  vector<Message> packetBuffer;
  mutex bufferMutex;
  condition_variable bufferCondition;

  bool isListening;
  std::thread listenerThread;

  std::unordered_map<std::string, ClientConnection> connectionTable;
  mutex tableMutex;

  sockaddr_in createSockAddr(const string &ipAddress, int port);

public:
  explicit TCPSocket(const string &ip, int port);
  ~TCPSocket();
  void bindSocket();
  void setBroadcast();
  void listen();

  std::string getClientKey(const string &ip, int port);
  void addNewConnection(const std::string &ip, int port);
  void deleteNewConnection(const std::string &ip, int port);

  // General
  void setStatus(TCPStatusEnum newState);
  TCPStatusEnum getStatus() ;

  // For server
  void setStatusConnection(TCPStatusEnum newState,string &ip, int port);
  std::string getStatusConnection(const string &ip, int port) ;

  void startListening();
  void stopListening();

  bool send(const string &destinationIP, int32_t destinationPort, void *data,
            uint32_t size);
  void sendSegment(const Segment &segment, const string &destinationIP,
                   uint16_t destinationPort);

  void produceBuffer();
  Message consumeBuffer(const string &filterIP = "", uint16_t filterPort = 0,
                        uint32_t filterSeqNum = 0, uint32_t filterAckNum = 0,
                        uint8_t filterFlags = 0, int timeout = 10);

  ConnectionResult sendBackN(uint8_t *dataStream, uint32_t dataSize,
                 const string &destIP, uint16_t destPort, uint32_t startingSeqNum, bool isFile, string fileFullName);
  ConnectionResult receiveBackN(vector<Segment> &resBuffer, string dest_ip, uint16_t dest_port, uint32_t seqNum);
  string concatenatePayloads(vector<Segment> &segments);

  // Client Role
  ConnectionResult findBroadcast(string dest_ip, uint16_t dest_port);
  ConnectionResult startHandshake(string dest_ip, uint16_t dest_port);
  ConnectionResult respondFin(string dest_ip, uint16_t dest_port, uint32_t seqNum,uint32_t ackNum, uint32_t recfin_seqnum); 

  // Server Role
  ConnectionResult respondHandshake(string dest_ip, uint16_t dest_port);
  ConnectionResult startFin(string dest_ip, uint16_t dest_port, uint32_t seqNum, uint32_t ackNum);
  ConnectionResult listenBroadcast();
  void close();
};

#endif