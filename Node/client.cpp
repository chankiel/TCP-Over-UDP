#include "client.hpp"
#include "../Socket/socket.hpp"
#include "../tools/fileHandler.hpp"
#include "../tools/tools.hpp"
#include <cstdint>
#include <cstdlib>
#include <pthread.h>
#include <random>
#include <stdexcept>
#include <string>

void Client::run()
{
  connection->listen();
  connection->startListening();
  try{
    ConnectionResult statusBroadcast =
        connection->findBroadcast("255.255.255.255", serverPort);

    ConnectionResult statusHandshake =
        connection->startHandshake(statusBroadcast.ip, statusBroadcast.port);

    vector<Segment> res;
    ConnectionResult statusReceive =
        connection->receiveBackN(res, statusBroadcast.ip, statusBroadcast.port,
                                statusHandshake.seqNum + 1);

    ConnectionResult statusFin =
        connection->respondFin(statusBroadcast.ip, statusBroadcast.port,
                  statusHandshake.seqNum, statusHandshake.ackNum, statusReceive.seqNum);

    if (res.back().flags.ece == 1)
    {
      string filePath = "";
      std::cout<<INPUT<<"Received a File. Specify the save path: ";
      std::cin>>filePath;

      std::string filename(reinterpret_cast<char *>(res.back().payload),
                          res.back().payloadSize);

      string fileFullPath = filePath + "/" + filename;

      res.pop_back();
      std::string result = connection->concatenatePayloads(res);

      convertFromStrToFile(fileFullPath, result);

      std::cout << OUT << "File saved to "+fileFullPath +". Terminating Client. Thank you!" << std::endl;
    }
    else
    {
      std::string result = connection->concatenatePayloads(res);
      std::cout << OUT << " String received from Server. Result: " << std::endl;
      std::cout << OUT <<" "<< result << std::endl;
      std::cout << OUT << " Terminating Client. Thank you!" << std::endl;
    }
  }catch (const std::runtime_error &e){
    commandLine('!', "[ERROR] " + brackets(status_strings[(int)connection->getStatus()]) + std::string(e.what()));
  }
  exit(0);
}
