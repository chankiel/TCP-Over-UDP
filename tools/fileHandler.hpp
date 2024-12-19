#ifndef fileSender_h
#define fileSender_h

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <bitset>
#include "../Node/server.hpp"

void convertToBinary(const std::string &inputFile,
                     const std::string &outputFile);
void convertToFileContentAndSetItem(const std::string &inputFile, Server &server);
void convertToBinaryAndSetItem(const std::string &inputFile, Server &server);

void convertFromBinary(const std::string &binaryFile,
                       const std::string &outputFile);

void convertFromStrToFile(const std::string &outputFile, const std::string &content);
#endif