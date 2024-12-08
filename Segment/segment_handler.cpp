#include "segment_handler.hpp"

SegmentHandler::SegmentHandler()
    : windowSize(5), currentSeqNum(0), currentAckNum(0), dataStream(nullptr),
      dataSize(0), dataIndex(0) {}

SegmentHandler::~SegmentHandler()
{
  for (Segment &seg : segmentBuffer)
  {
    delete[] seg.payload;
  }
}

void SegmentHandler::generateSegments(uint32_t startingSeqNum,
                                      uint16_t sourcePort, uint16_t destPort)
{
  for (Segment &seg : segmentBuffer)
  {
    delete[] seg.payload;
  }
  segmentBuffer.clear();
  uint32_t numSegments = (dataSize + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE;

  segmentBuffer.resize(numSegments);
  uint32_t iterator = 0;
  for (uint32_t i = 0; i < numSegments; i++)
  {
    uint32_t payloadSize =
        min(MAX_PAYLOAD_SIZE, dataSize - i * MAX_PAYLOAD_SIZE);

    segmentBuffer.emplace_back();
    Segment &seg = segmentBuffer.back();

    seg.payload = new uint8_t[payloadSize];
    memcpy(seg.payload,
           static_cast<uint8_t *>(dataStream) + iterator,
           payloadSize);
    seg.window = windowSize;
    seg.payloadSize = payloadSize;
    seg.seqNum = startingSeqNum + i;
    seg.sourcePort = sourcePort;
    seg.destPort = destPort;

    iterator += payloadSize;
    // while (seg.checksum == 0)
    // {
    updateChecksum(seg);
    // }
  }

  dataIndex = numSegments - 1;

  cout << "NumSegments: " << numSegments << endl;
}

void SegmentHandler::setDataStream(uint8_t *dataStream, uint32_t dataSize,
                                   uint32_t startingSeqNum, uint16_t sourcePort,
                                   uint16_t destPort)
{
  this->dataStream = dataStream;
  this->dataSize = dataSize;
  currentSeqNum = startingSeqNum - 1;
  currentAckNum = startingSeqNum - 1;

  generateSegments(startingSeqNum, sourcePort, destPort);
}

uint8_t SegmentHandler::getWindowSize() { return this->windowSize; }

Segment *SegmentHandler::advanceWindow(uint8_t size)
{
  lock_guard<mutex> lock(mtx);
  if (dataIndex + 1 >= segmentBuffer.size())
  {
    return nullptr;
  }
  else
  {
    currentSeqNum += size;
    dataIndex += 1;
  }

  return &segmentBuffer[dataIndex];
}

void SegmentHandler::ackWindow(uint32_t seqNum)
{
  lock_guard<mutex> lock(mtx);
  if (seqNum > currentAckNum)
  {
    currentAckNum = seqNum;
  }
}

uint32_t SegmentHandler::getCurrentSeqNum()
{
  lock_guard<mutex> lock(mtx);
  return currentSeqNum;
}

uint32_t SegmentHandler::getCurrentAckNum()
{
  lock_guard<mutex> lock(mtx);
  return currentAckNum;
}

void SegmentHandler::goBackWindow()
{
  lock_guard<mutex> lock(mtx);
  dataIndex -= (currentSeqNum - currentAckNum);
  currentSeqNum = currentAckNum;
}

bool SegmentHandler::isFinished(uint32_t startingSeqNum)
{
  lock_guard<mutex> lock(mtx);
  return currentAckNum - startingSeqNum + 1 == (segmentBuffer.size() + 1) / 2;
}

void SegmentHandler::addMetadata(string fileFullName, uint16_t sourcePort, uint16_t destPort)
{
  Segment seg = createSegment(fileFullName, sourcePort, destPort);
  seg.payloadSize = static_cast<uint16_t>(fileFullName.length());
  seg.window = windowSize;
  seg.seqNum = segmentBuffer.back().seqNum + 1;
  seg.flags.ece = 1;
  segmentBuffer.push_back(seg);
}

void SegmentHandler::markEOF()
{
  segmentBuffer.back().flags.psh = 1;
}