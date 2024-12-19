#include "server.hpp"
#include "../tools/tools.hpp"
#include <stdexcept>
#include <string>

void Server::run()
{
  connection->listen();
  connection->startListening();

  vector<thread> threads;
  while (true)
  {
    try
    {
      ConnectionResult statusBroadcast = connection->listenBroadcast();
      threads.emplace_back([statusBroadcast, this]()
                           {
        connection->addNewConnection(statusBroadcast.ip,statusBroadcast.port);
        std::cout<<statusBroadcast.ip<<":"<<statusBroadcast.port<<endl;
        try{
          ConnectionResult statusHandshake = connection->respondHandshake(statusBroadcast.ip, statusBroadcast.port);
          bool isFile = true;
          std::string fileFullName;
          if (fileEx == "-1")
          {
            isFile = false;
          }
          else
          {
            fileFullName = fileEx.empty() ? fileName : fileName + "." + fileEx;
          }

          ConnectionResult statusSend = connection->sendBackN(
              reinterpret_cast<uint8_t *>(item.data()),
              static_cast<uint32_t>(item.length()),
              statusBroadcast.ip,
              statusBroadcast.port,
              statusHandshake.ackNum,
              isFile,
              fileFullName);

          ConnectionResult statusFin = connection->startFin(
              statusBroadcast.ip,
              statusBroadcast.port,
              statusHandshake.seqNum,
              statusHandshake.ackNum);
        }catch (const std::runtime_error &e){
          commandLine('!', "[ERROR] " + brackets(status_strings[(int)connection->getStatus()]) + std::string(e.what()));
        } 
        
        connection->deleteNewConnection(statusBroadcast.ip,statusBroadcast.port);
        });
    }
    catch (const std::runtime_error &e)
    {
      commandLine('!', "[ERROR] " + brackets(status_strings[(int)connection->getStatus()]) + std::string(e.what()));
    }
  }
  for (auto &t : threads)
  {
    if (t.joinable())
    {
      t.join();
    }
  }
}
