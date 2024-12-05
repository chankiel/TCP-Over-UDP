#ifndef segment_h
#define segment_h

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <iomanip>
#include <utility>
#include <vector>
using namespace std;

struct Segment
{
  uint16_t sourcePort;
  uint16_t destPort;
  uint32_t seqNum;
  uint32_t ackNum;
  // todo continue

  struct
  {
    unsigned int data_offset : 4;
    unsigned int reserved : 4;
  };

  struct
  {
    unsigned int cwr : 1;
    unsigned int ece : 1;
    unsigned int urg : 1;
    unsigned int ack : 1;
    unsigned int psh : 1;
    unsigned int rst : 1;
    unsigned int syn : 1;
    unsigned int fin : 1;
  } flags;

  uint16_t window;
  uint16_t checksum;
  uint16_t urgPointer;
  uint32_t payloadSize;
  uint8_t *payload;
  Segment()
      : sourcePort(0),
        destPort(0),
        seqNum(0),
        ackNum(0),
        window(0),
        checksum(0),
        urgPointer(0),
        payloadSize(0),
        payload(nullptr)
  {
    data_offset = 6;
    reserved = 0;
    memset(&flags, 0, sizeof(flags));
  }

  Segment(const Segment &other)
      : sourcePort(other.sourcePort),
        destPort(other.destPort),
        seqNum(other.seqNum),
        ackNum(other.ackNum),
        window(other.window),
        checksum(other.checksum),
        urgPointer(other.urgPointer),
        payloadSize(other.payloadSize),
        payload(nullptr) 
  {
    if (other.payload != nullptr && other.payloadSize > 0)
    {
      payload = new uint8_t[other.payloadSize];
      std::copy(other.payload, other.payload + other.payloadSize, payload);
    }

    data_offset = other.data_offset;
    reserved = other.reserved;
    flags = other.flags;
  }
} __attribute__((packed));

const uint8_t FIN_FLAG = 1;
const uint8_t SYN_FLAG = 2;
const uint8_t ACK_FLAG = 16;
const uint8_t SYN_ACK_FLAG = SYN_FLAG | ACK_FLAG;
const uint8_t FIN_ACK_FLAG = FIN_FLAG | ACK_FLAG;

// Payload size di options 32 bit
const uint32_t HEADER_SIZE = 24;
const uint32_t MAX_PAYLOAD_SIZE = 1476;
const uint32_t MAX_SEGMENT_SIZE = HEADER_SIZE + MAX_PAYLOAD_SIZE; // MTU: 1500

/**
 * Generate Segment that contain SYN packet
 */
Segment syn(uint32_t seqNum);

/**
 * Generate Segment that contain ACK packet
 */
Segment ack(uint32_t seqNum, uint32_t ackNum);

/**
 * Generate Segment that contain SYN-ACK packet
 */
Segment synAck(uint32_t seqNum);

/**
 * Generate Segment that contain FIN packet
 */
Segment fin();

/**
 * Generate Segment that contain FIN-ACK packet
 */
Segment finAck();

// update return type as needed
uint16_t calculateChecksum(Segment &segment);

/**
 * Return a new segment with a calcuated checksum fields
 */
Segment updateChecksum(Segment segment);

/**
 * Check if a TCP Segment has a valid checksum
 */
bool isValidChecksum(Segment segment);

/**
 * Custom constructor Segment
 */
Segment createSegment(const string &data, uint16_t sport, uint16_t dport);

/**
 * Deep copy Segment
 */
Segment copySegment(const Segment &source);

/**
 * Print segment info for debug
 */
void printSegment(const Segment &segment);

/**
 * Operator overloading for comparison
 */
bool operator==(const Segment &lhs, const Segment &rhs);

/**
 * Encoding Segment to Buffer for Transmitting
 */
void encodeSegment(const Segment &segment, uint8_t *buffer);

/**
 * Decode Buffer received to Segment
 */
Segment decodeSegment(const uint8_t *buffer, uint32_t length);

#endif