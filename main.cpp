#include <iostream>
#include <cstring>
#include <cstdint>
#include <arpa/inet.h>  // For htons, htonl
using namespace std;

struct Segment
{
    uint16_t sourcePort: 16;
    uint16_t destPort: 16;
    uint32_t seqNum: 32;
    uint32_t ackNum: 32;

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

    uint16_t window: 16;
    uint16_t checksum: 16;
    uint16_t urgPointer: 16;
    uint8_t *payload;

    Segment() : sourcePort(0), destPort(0), seqNum(0), ackNum(0), window(0),
                checksum(0), urgPointer(0), payload(nullptr)
    {
        data_offset = 5;
        reserved = 0;
        memset(&flags, 0, sizeof(flags));
    }
} __attribute__((packed));

void printBinary(uint16_t value) {
    for (int i = 15; i >= 0; --i) {
        cout << ((value >> i) & 1);
    }
}

uint8_t *calculateChecksum(Segment segment)
{
    segment.checksum = 0; // Ensure the checksum field is zero during calculation

    // Buffer for checksum calculation
    size_t segmentSize = sizeof(Segment);
    size_t payloadSize = (segment.data_offset - 5) * 4; // Calculate payload size
    size_t totalSize = segmentSize + payloadSize;

    uint8_t *buffer = new uint8_t[totalSize];
    memcpy(buffer, &segment, segmentSize);

    if (segment.payload != nullptr && payloadSize > 0)
    {
        memcpy(buffer + segmentSize, segment.payload, payloadSize);
    }

    // Compute checksum
    uint32_t sum = 0;
    for (size_t i = 0; i < totalSize; i += 2)
    {
        uint16_t word = (buffer[i] << 8);
        if (i + 1 < totalSize)
        {
            word |= buffer[i + 1];
        }
        sum += word;

        // Handle carry
        while (sum >> 16)
        {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
    }

    // Finalize checksum
    uint16_t checksum = ~sum;

    // Prepare result
    uint8_t *result = new uint8_t[2];
    result[0] = (checksum >> 8) & 0xFF; // High byte
    result[1] = checksum & 0xFF;        // Low byte

    delete[] buffer; // Free buffer memory
    return result;
}

int main()
{
    // Create and initialize a Segment
    Segment segment;
    segment.sourcePort = htons(12345);  // Use htons to convert port to network byte order
    segment.destPort = htons(80);       // Use htons to convert port to network byte order
    segment.seqNum = htonl(1);          // Use htonl to convert to network byte order
    segment.ackNum = htonl(0);          // Use htonl to convert to network byte order
    segment.window = htons(8192);      // Use htons to convert to network byte order
    segment.payload = new uint8_t[10]{'H', 'e', 'l', 'l', 'o', ' ', 'T', 'C', 'P', '!'};
    segment.data_offset = 5 + 10 / 4; // Header + payload size (in 32-bit words)

    // Calculate checksum
    uint8_t *checksum = calculateChecksum(segment);

    // Print checksum in binary
    cout << "Checksum: ";
    printBinary(checksum[0]);
    printBinary(checksum[1]);
    cout << endl;

    // Clean up
    delete[] checksum;
    delete[] segment.payload;

    return 0;
}
