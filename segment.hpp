#ifndef segment_h
#define segment_h

#include <cstdint>

struct Segment
{
    uint16_t sourcePort: 16;
    uint16_t destPort: 16;
    uint32_t seqNum: 32;
    uint32_t ackNum: 32;
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
        // todo continue ...
    } flags;

    uint16_t window: 16;
    uint16_t checksum: 16;
    uint16_t urgPointer: 16;
    // todo continue
    uint8_t *payload;
    Segment() : sourcePort(0), destPort(0), seqNum(0), ackNum(0), window(0),
                checksum(0), urgPointer(0), payload(nullptr)
    {
        data_offset = 5;
        reserved = 0;
        memset(&flags, 0, sizeof(flags));
    }
} __attribute__((packed));

const uint8_t FIN_FLAG = 1;
const uint8_t SYN_FLAG = 2;
const uint8_t ACK_FLAG = 16;
const uint8_t SYN_ACK_FLAG = SYN_FLAG | ACK_FLAG;
const uint8_t FIN_ACK_FLAG = FIN_FLAG | ACK_FLAG;

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
uint8_t *calculateChecksum(Segment segment);

/**
 * Return a new segment with a calcuated checksum fields
 */
Segment updateChecksum(Segment segment);

/**
 * Check if a TCP Segment has a valid checksum
 */
bool isValidChecksum(Segment segment);

#endif