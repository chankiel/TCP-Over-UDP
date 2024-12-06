#ifndef segment_handler_h
#define segment_handler_h

#include "segment.hpp"
#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <mutex>
#include "../Message/message.hpp"
using namespace std;

class SegmentHandler
{
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
    void generateSegments();

public:
    SegmentHandler();
    ~SegmentHandler();
    void setDataStream(uint8_t *dataStream, uint32_t dataSize);
    uint8_t getWindowSize();
    Segment *advanceWindow(uint8_t size);
    void ackWindow(uint8_t size);
    uint32_t getCurrentSeqNum();
    uint32_t getCurrentAckNum();

    void addStatusSegment(uint8_t handshakeType, uint32_t seqNum, uint32_t ackNum);
};

#endif