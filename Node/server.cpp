#include "server.hpp"
#include "../tools/tools.hpp"
#include <stdexcept>
int BROADCAST_TIMEOUT = 30; // temporary
int MAX_TRY = 10;

ConnectionResult Server::listenBroadcast() {
  for (int i = 0; i < MAX_TRY; i++) {
    try {
      Message answer =
          connection->consumeBuffer("", 0, 0, 0, BROADCAST_TIMEOUT);
      commandLine('+', "Received Broadcast Message\n");
      Segment temp = accBroad();
      connection->sendSegment(temp, answer.ip, answer.port);
      return ConnectionResult(1, answer.ip, answer.port, answer.segment.seqNum,
                              answer.segment.ackNum);
    } catch (runtime_error()) {
      commandLine('x', "Timeout " + std::to_string(i + 1) + "\n");
      continue;
    }
  }
  return ConnectionResult(-1, 0, 0, 0, 0);
}