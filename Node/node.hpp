#ifndef node_h
#define node_h

#include "../Socket/socket.hpp"

/**
 * Abstract class.
 *
 * This is the base class for Server and Client class.
 */
class Node
{
protected:
  string ip;
  string item;
  string fileName;
  string fileEx;
  uint16_t port;
  TCPSocket *connection;

public:
  Node(string ip, uint16_t port);
  ~Node();
  virtual void run() = 0;
  void setItem(const std::string &string);
  void setItemFromBin(const std::string &binaryString);

  // Implemented in Header
  std::string getItem() const { return item; }
  std::string getFileName() const { return fileName; }
  void setFileName(const std::string &name) { fileName = name; }
  std::string getFileEx() const { return fileEx; }
  void setFileEx(const std::string &extension) { fileEx = extension; }
};

#endif