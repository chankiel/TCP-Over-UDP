#ifndef segment_handler_h
#define segment_handler_h

#include "segment.hpp"
#include <vector>
#include <cmath>

using namespace std;

class SegmentHandler
{
private:
    uint8_t windowSize;
    uint32_t currentSeqNum;
    uint32_t currentAckNum;
    void *dataStream;
    uint32_t dataSize;
    uint32_t dataIndex;
    vector<Segment> segmentBuffer; // or use std vector if you like

    // Ubah dataStream jadi segment2
    void generateSegments();

public:
    SegmentHandler();
    ~SegmentHandler();
    void setDataStream(uint8_t *dataStream, uint32_t dataSize);
    uint8_t getWindowSize();

    // Size: jumlah segment acknowledged
    Segment* advanceWindow(uint8_t size);
};

#endif