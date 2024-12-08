#ifndef fileReceiver_h
#define fileReceiver_h

#include <iostream>
#include <fstream>
#include <bitset>
#include <sstream>

void convertFromBinary(const std::string &binaryFile,
                       const std::string &outputFile);

void convertFromStrToFile(const std::string &outputFile, const std::string &content);
#endif