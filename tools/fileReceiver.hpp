#ifndef fileReceiver_h
#define fileReceiver_h

#include <iostream>
#include <fstream>
#include <bitset>
#include <sstream>

void convertFromBinary(const std::string &binaryFile,
                       const std::string &outputFile);

void convertFromClientToFile(const std::string &outputFile, Client &client);
void convertFromServerToFile(const std::string &outputFile, Server &server); // TESTING PURPOSES
#endif