#include "segment_handler.hpp"

SegmentHandler::SegmentHandler() : windowSize(5 * MAX_SEGMENT_SIZE), currentSeqNum(0), currentAckNum(0), dataStream(nullptr), dataSize(0), dataIndex(0) {}

SegmentHandler::~SegmentHandler()
{
}

void SegmentHandler::generateSegments()
{
    // Hitung jumlah segment, hasil floor dataSize / MAX_PAYLOAD_SIZE
    uint32_t numSegments = (dataSize + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE;
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
        seg.payloadSize = payloadSize;
        seg.checksum = 0;
        seg.seqNum = currentSeqNum;
        currentSeqNum += payloadSize;
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
    currentSeqNum += size;
    dataIndex += 1;

    if (dataIndex >= segmentBuffer.size())
    {
        return nullptr;
    }

    return &segmentBuffer[dataIndex];
}

void SegmentHandler::ackWindow(uint8_t size)
{
    lock_guard<mutex> lock(mtx);
    currentAckNum += size;
}

uint32_t SegmentHandler::getCurrentSeqNum()
{
    return currentSeqNum;
}

uint32_t SegmentHandler::getCurrentAckNum()
{
    return currentAckNum;
}