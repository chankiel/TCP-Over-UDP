#ifndef fileSender_h
#define fileSender_h

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <bitset>

void convertToBinary(const std::string &inputFile,
                     const std::string &outputFile);
void convertToFileContentAndSetItem(const std::string &inputFile, Server &server);
void convertToBinaryAndSetItem(const std::string &inputFile, Server &server);
#endif