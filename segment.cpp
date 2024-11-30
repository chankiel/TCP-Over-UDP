#include <cstdint>
#include <cstring>

/**
 * Generate a Segment containing a SYN packet
 */
Segment syn(uint32_t seqNum)
{
    Segment segment;
    segment.seqNum = seqNum;
    segment.flags.syn = 1;
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
    return segment;
}

/**
 * Generate a Segment containing a FIN packet
 */
Segment fin()
{
    Segment segment;
    segment.flags.fin = 1;
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
    return segment;
}

/**
 * Calculate the checksum for a given Segment
 */
uint8_t *calculateChecksum(Segment segment)
{
    // TODO: buat checksum
    return 0;
}

/**
 * Update a Segment with the calculated checksum
 */
Segment updateChecksum(Segment segment)
{
    auto checksum = calculateChecksum(segment);
    segment.checksum = (checksum[0] << 8) | checksum[1];
    return segment;
}

/**
 * Verify if a Segment has a valid checksum
 */
bool isValidChecksum(Segment segment)
{
    auto checksum = calculateChecksum(segment);
    uint16_t computed = (checksum[0] << 8) | checksum[1];
    return computed == segment.checksum;
}

