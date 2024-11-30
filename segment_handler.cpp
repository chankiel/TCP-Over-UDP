#include "segment_handler.hpp"
#include <iostream>
using namespace std;

SegmentHandler::SegmentHandler() : windowSize(5), currentSeqNum(0), currentAckNum(0), dataStream(nullptr), dataSize(0), dataIndex(0) {}

SegmentHandler::~SegmentHandler()
{
    for (auto &seg : segmentBuffer)
    {
        delete[] seg.payload;
    }
}

void SegmentHandler::generateSegments()
{
    // Sesuai MTU = 1460
    const uint32_t MAX_PAYLOAD_SIZE = 10;

    // Hitung jumlah segment, hasil floor dataSize / MAX_PAYLOAD_SIZE
    int numSegments = (dataSize + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE;
    segmentBuffer.resize(numSegments);

    // Iterasi untuk buat segment2
    for (uint32_t i = 0; i < numSegments; i++)
    {
        Segment &seg = segmentBuffer[i];

        // Tentuin payload size
        uint32_t payloadSize = min(MAX_PAYLOAD_SIZE, dataSize - i * MAX_PAYLOAD_SIZE);

        // Copy isi dari dataStream ke payload sesuai payloadSize
        seg.payload = new uint8_t[payloadSize];
        memcpy(seg.payload, static_cast<uint8_t *>(dataStream) + i * MAX_PAYLOAD_SIZE, payloadSize);
        seg.window = windowSize;
        seg.checksum = 0;
        seg.seqNum = currentSeqNum;
        currentSeqNum += payloadSize;

        // cout<<static_cast<uint8_t *>(dataStream) + i * MAX_PAYLOAD_SIZE<<endl;
    }
}

void SegmentHandler::setDataStream(uint8_t *dataStream, uint32_t dataSize)
{
    this->dataStream = dataStream;
    this->dataSize = dataSize;
    this->dataIndex = 0;
    currentSeqNum = 0;
    currentAckNum = 0;

    generateSegments();
}

uint8_t SegmentHandler::getWindowSize()
{
    return this->windowSize;
}

Segment *SegmentHandler::advanceWindow(uint8_t size)
{
    dataIndex += size;

    if (dataIndex >= segmentBuffer.size())
    {
        cout<<dataIndex<<endl;
        return nullptr;
    }

    return &segmentBuffer[dataIndex];
}