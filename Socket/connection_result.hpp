#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <bits/stdc++.h>

using namespace std;

class ConnectionResult
{
public:
    bool success;
    string ip;
    uint16_t port;
    uint32_t seqNum;
    uint32_t ackNum;

    ConnectionResult(bool success, string ip, uint16_t port, uint32_t seqNum, uint32_t ackNum) : success(success), ip(ip), port(port), seqNum(seqNum), ackNum(ackNum) {}
    ConnectionResult(): ip(""),port(0),seqNum(0),ackNum(0){}
};

#endif