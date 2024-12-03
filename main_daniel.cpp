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

    cout << "Buffer Data (Hex): ";
    for (size_t i = 0; i < totalSize; ++i) {
        printf("%02x ", buffer[i]);
    }
    cout << endl;

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

int main_daniel()
{
    Segment segment;
    segment.sourcePort = htons(12345);
    segment.destPort = htons(80);     
    segment.seqNum = htonl(1);        
    segment.ackNum = htonl(0);        
    segment.window = htons(8192);    
    segment.payload = new uint8_t[10]{'H', 'e', 'l', 'l', 'o', ' ', 'T', 'C', 'P', '!'};
    segment.data_offset = 5 + 10 / 4; 

    cout << "sourcePort: " << segment.sourcePort << endl;
    cout << "destPort: " << segment.destPort << endl;
    cout << "seqNum: " << segment.seqNum << endl;
    cout << "ackNum: " << segment.ackNum << endl;
    cout << "window: " << segment.window << endl;
    cout << "data_offset: " << segment.data_offset << endl;

    cout << "Payload: ";
    for (int i = 0; i < 10; ++i) {
        cout << segment.payload[i] << " ";
    }
    cout << endl;

    uint16_t checksum = calculateChecksum(segment);
    
    cout << "Checksum: ";
    printBinary(checksum);
    cout << endl;

    delete[] segment.payload;

    return 0;
}
