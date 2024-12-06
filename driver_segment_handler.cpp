#include "segment_handler.hpp"
#include <iostream>
using namespace std;

int main_driver()
{
    uint8_t data[] = "Hello, this is a test data stream for TCP segment handling.";
    uint32_t dataSize = sizeof(data) - 1; // Exclude null terminator

    SegmentHandler handler;

    handler.setDataStream(data, dataSize);

    uint8_t windowSize = handler.getWindowSize();
    int acknowledged = 0;

     for (uint8_t i = 0; i < windowSize; ++i) {
        Segment *seg = handler.advanceWindow(windowSize);
        if (seg) {
            cout<<seg->payload<<endl;
        }
    }

    return 0;
}
