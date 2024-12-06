#include "segment.hpp"
#include <utility> // Required for std::pair
#include <string>  // Required for std::string
#include <vector>

/**
 * Generate a Segment containing a SYN packet
 */
Segment syn(uint32_t seqNum)
{
  Segment segment;
  segment.seqNum = seqNum;
  segment.flags.syn = 1;
  segment.data_offset = 6;
  segment.payloadSize = 0;
  return segment;
}

/**
 * Generate a Segment containing an ACK packet
 */
Segment ack(uint32_t seqNum, uint32_t ackNum)
{
  Segment segment;
  segment.seqNum = seqNum;
  segment.ackNum = ackNum;
  segment.flags.ack = 1;
  segment.payloadSize = 0;
  return segment;
}

/**
 * Generate a Segment containing a SYN-ACK packet
 */
Segment synAck(uint32_t seqNum)
{
  Segment segment;
  segment.seqNum = seqNum;
  segment.flags.syn = 1;
  segment.flags.ack = 1;
  segment.payloadSize = 0;
  return segment;
}

/**
 * Generate a Segment containing a FIN packet
 */
Segment fin()
{
  Segment segment;
  segment.flags.fin = 1;
  segment.payloadSize = 0;
  return segment;
}

/**
 * Generate a Segment containing a FIN-ACK packet
 */
Segment finAck()
{
  Segment segment;
  segment.flags.fin = 1;
  segment.flags.ack = 1;
  segment.payloadSize = 0;
  return segment;
}

/**
 * Calculate the checksum for a given Segment
 */
uint16_t calculateChecksum(Segment &segment)
{
  segment.checksum = 0;

  const size_t segmentSize = sizeof(Segment);
  const size_t payloadSize = (segment.data_offset - 5) * 4;
  const size_t totalSize = segmentSize + payloadSize;

  uint8_t buffer[totalSize];
  memset(buffer, 0, totalSize);

  memcpy(buffer, &segment, segmentSize);

  if (segment.payload != nullptr && payloadSize > 0)
  {
    memcpy(buffer + segmentSize, segment.payload, payloadSize);
  }

  uint32_t sum = 0;
  for (size_t i = 0; i < totalSize; i += 2)
  {
    uint16_t word = (buffer[i] << 8);
    if (i + 1 < totalSize)
    {
      word |= buffer[i + 1];
    }
    sum += word;

    while (sum >> 16)
    {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }
  }

  uint16_t checksum = ~sum;

  return checksum;
}

/**
 * Update a Segment with the calculated checksum
 */
Segment updateChecksum(Segment segment)
{
  auto checksum = calculateChecksum(segment);
  segment.checksum = checksum;
  return segment;
}

/**
 * Verify if a Segment has a valid checksum
 */
bool isValidChecksum(Segment segment)
{
  auto checksum = calculateChecksum(segment);
  uint16_t computed = checksum;
  return computed == segment.checksum;
}

Segment createSegment(const std::string &data, uint16_t sport, uint16_t dport)
{
  Segment segment;

  segment.payloadSize = static_cast<uint16_t>(data.length());
  if (segment.payloadSize == 0)
  {
    segment.payload = nullptr;
  }
  else
  {
    segment.payload = new uint8_t[segment.payloadSize];
    std::memcpy(segment.payload, data.c_str(), segment.payloadSize);
  }

  segment.sourcePort = sport;
  segment.destPort = dport;
  return segment;
}

void printSegment(const Segment &segment)
{
  std::cout << "Segment Debug Information:\n";
  std::cout << "-------------------------------------\n";
  std::cout << "Source Port:      " << segment.sourcePort << "\n";
  std::cout << "Destination Port: " << segment.destPort << "\n";
  std::cout << "Sequence Number:  " << segment.seqNum << "\n";
  std::cout << "Acknowledgment #: " << segment.ackNum << "\n";
  std::cout << "Data Offset:      " << static_cast<int>(segment.data_offset) << "\n";
  std::cout << "Reserved:         " << static_cast<int>(segment.reserved) << "\n";
  std::cout << "Flags:\n";
  std::cout << "  CWR: " << segment.flags.cwr
            << ", ECE: " << segment.flags.ece
            << ", URG: " << segment.flags.urg
            << ", ACK: " << segment.flags.ack
            << ", PSH: " << segment.flags.psh
            << ", RST: " << segment.flags.rst
            << ", SYN: " << segment.flags.syn
            << ", FIN: " << segment.flags.fin << "\n";
  std::cout << "Window Size:      " << segment.window << "\n";
  std::cout << "Checksum:         " << segment.checksum << "\n";
  std::cout << "Urgent Pointer:   " << segment.urgPointer << "\n";
  std::cout << "Payload Size:     " << segment.payloadSize << " bytes\n";
  if (segment.payload != nullptr)
  {
    std::cout << "Payload Data:     ";
    for (uint32_t i = 0; i < segment.payloadSize; ++i)
    {
      std::cout << std::hex << std::setfill('0') << std::setw(2)
                << static_cast<int>(segment.payload[i]) << " ";
    }
    std::cout << std::dec << "\n";
  }
  else
  {
    std::cout << "Payload Data:     (null)\n";
  }
  std::cout << "-------------------------------------\n";
}

bool operator==(const Segment &lhs, const Segment &rhs)
{
  // Compare header
  if (lhs.sourcePort != rhs.sourcePort ||
      lhs.destPort != rhs.destPort ||
      lhs.seqNum != rhs.seqNum ||
      lhs.ackNum != rhs.ackNum ||
      lhs.data_offset != rhs.data_offset ||
      lhs.reserved != rhs.reserved ||
      lhs.flags.cwr != rhs.flags.cwr ||
      lhs.flags.ece != rhs.flags.ece ||
      lhs.flags.urg != rhs.flags.urg ||
      lhs.flags.ack != rhs.flags.ack ||
      lhs.flags.psh != rhs.flags.psh ||
      lhs.flags.rst != rhs.flags.rst ||
      lhs.flags.syn != rhs.flags.syn ||
      lhs.flags.fin != rhs.flags.fin ||
      lhs.window != rhs.window ||
      lhs.checksum != rhs.checksum ||
      lhs.urgPointer != rhs.urgPointer ||
      lhs.payloadSize != rhs.payloadSize)
  {
    return false;
  }

  // Compare payload data
  if (lhs.payload != nullptr && rhs.payload != nullptr)
  {
    if (std::memcmp(lhs.payload, rhs.payload, lhs.payloadSize) != 0)
    {
      return false;
    }
  }
  else if (lhs.payload != rhs.payload)
  {
    return false;
  }

  return true;
}

Segment copySegment(const Segment &source)
{
  Segment copy = {};

  copy.sourcePort = source.sourcePort;
  copy.destPort = source.destPort;
  copy.seqNum = source.seqNum;
  copy.ackNum = source.ackNum;
  copy.data_offset = source.data_offset;
  copy.reserved = source.reserved;
  copy.flags.cwr = source.flags.cwr;
  copy.flags.ece = source.flags.ece;
  copy.flags.urg = source.flags.urg;
  copy.flags.ack = source.flags.ack;
  copy.flags.psh = source.flags.psh;
  copy.flags.rst = source.flags.rst;
  copy.flags.syn = source.flags.syn;
  copy.flags.fin = source.flags.fin;
  copy.window = source.window;
  copy.checksum = source.checksum;
  copy.urgPointer = source.urgPointer;
  copy.payloadSize = source.payloadSize;

  if (source.payload != nullptr && source.payloadSize > 0)
  {
    copy.payload = new uint8_t[source.payloadSize];
    memcpy(copy.payload, source.payload, source.payloadSize);
  }
  else
  {
    copy.payload = nullptr;
  }

  return copy;
}

void encodeSegment(const Segment &segment, uint8_t *buffer)
{
  memcpy(buffer, &segment.sourcePort, sizeof(segment.sourcePort));
  memcpy(buffer + 2, &segment.destPort, sizeof(segment.destPort));
  memcpy(buffer + 4, &segment.seqNum, sizeof(segment.seqNum));
  memcpy(buffer + 8, &segment.ackNum, sizeof(segment.ackNum));

  buffer[12] = (segment.data_offset << 4) | (segment.reserved & 0x0F);

  memcpy(buffer + 13, &segment.flags, sizeof(segment.flags));
  memcpy(buffer + 14, &segment.window, sizeof(segment.window));
  memcpy(buffer + 16, &segment.checksum, sizeof(segment.checksum));
  memcpy(buffer + 18, &segment.urgPointer, sizeof(segment.urgPointer));
  memcpy(buffer + 20, &segment.payloadSize, sizeof(segment.payloadSize));
  memcpy(buffer + 24, segment.payload, segment.payloadSize);
}

Segment decodeSegment(const uint8_t *buffer, uint32_t length)
{
  Segment segment = {};
  memcpy(&segment.sourcePort, buffer, sizeof(segment.sourcePort));
  memcpy(&segment.destPort, buffer + 2, sizeof(segment.destPort));
  memcpy(&segment.seqNum, buffer + 4, sizeof(segment.seqNum));
  memcpy(&segment.ackNum, buffer + 8, sizeof(segment.ackNum));

  segment.data_offset = (buffer[12] >> 4) & 0x0F;
  segment.reserved = buffer[12] & 0x0F;

  memcpy(&segment.flags, buffer + 13, sizeof(segment.flags));
  memcpy(&segment.window, buffer + 14, sizeof(segment.window));
  memcpy(&segment.checksum, buffer + 16, sizeof(segment.checksum));
  memcpy(&segment.urgPointer, buffer + 18, sizeof(segment.urgPointer));
  memcpy(&segment.payloadSize, buffer + 20, sizeof(segment.payloadSize));

  if (segment.payloadSize == 0)
  {
    segment.payload = nullptr;
  }
  else
  {
    segment.payload = new uint8_t[segment.payloadSize];
    memcpy(segment.payload, buffer + 24, segment.payloadSize);
  }
  return segment;
}