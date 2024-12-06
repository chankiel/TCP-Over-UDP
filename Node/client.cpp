#include "client.hpp"
#include "../tools/tools.hpp"
#include <stdexcept>
#include <string>
int BROADCAST_TIMEOUT = 30; // temporary
int MAX_TRY = 10;

ConnectionResult Client::findBroadcast(string dest_ip, uint16_t dest_port) {
  for (int i = 0; i < MAX_TRY; i++) {
    try {
      Segment temp = broad();

      connection->sendSegment(temp, dest_ip, dest_port);
      commandLine('i', "Sending Broadcast\n");
      Message answer =
          connection->consumeBuffer("", 0, 0, 0, 255, BROADCAST_TIMEOUT);
      commandLine('i', "Someone received the broadcast\n");
      return ConnectionResult(1, answer.ip, answer.port, answer.segment.seqNum,
                              answer.segment.ackNum);
    } catch (runtime_error()) {
      commandLine('x', "Timeout " + std::to_string(i + 1) + "\n");
      continue;
    }
  }
  return ConnectionResult(-1, 0, 0, 0, 0);
}
