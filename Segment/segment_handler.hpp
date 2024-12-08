#ifndef segment_handler_h
#define segment_handler_h

#include "segment.hpp"
#include <cmath>
#include <cstring>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;

class SegmentHandler {
private:
  uint32_t windowSize;
  uint32_t currentSeqNum;
  uint32_t currentAckNum;
  void *dataStream;
  uint32_t dataSize;
  uint32_t dataIndex;
  mutex mtx;
  vector<Segment> segmentBuffer; // or use std vector if you like

  // Ubah dataStream jadi segment2
  void generateSegments(uint32_t startingSeqNum,uint16_t sourcePort,uint16_t destPort);

public:
  SegmentHandler();
  ~SegmentHandler();
  void setDataStream(uint8_t *dataStream, uint32_t dataSize, uint32_t startingSeqNum, uint16_t sourcePort,uint16_t destPort);
  uint8_t getWindowSize();
  Segment *advanceWindow(uint8_t size);
  void ackWindow(uint32_t seqNum);
  uint32_t getCurrentSeqNum();
  uint32_t getCurrentAckNum();
  void goBackWindow();
  bool isFinished(uint32_t startingSeqNum);
  void addMetadata(string fileFullName, uint16_t sourcePort, uint16_t destPort);
  void markEOF();
};

#endif