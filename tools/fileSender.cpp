#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <bitset>
#include "../Node/server.hpp"

void convertToBinary(const std::string &inputFile, const std::string &outputFile)
{
  std::ifstream input(inputFile, std::ios::binary);
  if (!input)
  {
    std::cerr << "Error: Unable to open input file: " << inputFile << std::endl;
    return;
  }

  std::ofstream output(outputFile, std::ios::binary);
  if (!output)
  {
    std::cerr << "Error: Unable to open output file: " << outputFile << std::endl;
    return;
  }

  char byte;
  while (input.read(&byte, sizeof(byte)))
  {
    std::bitset<8> binary(byte);
    output << binary.to_string();
  }

  input.close();
  output.close();

  std::cout << "Conversion complete. Binary data written to " << outputFile << std::endl;
}

void convertToFileContentAndSetItem(const std::string &inputFile, Server &server)
{
  std::ifstream input(inputFile);
  if (!input)
  {
    std::cerr << "Error: Unable to open input file: " << inputFile << std::endl;
    return;
  }

  std::string fileContent((std::istreambuf_iterator<char>(input)),
                          std::istreambuf_iterator<char>());

  server.setItem(fileContent);

  std::cout << "File content successfully set in the server item." << std::endl;

  input.close();
}

void convertToBinaryAndSetItem(const std::string &inputFile, Server &server)
{
  std::ifstream input(inputFile, std::ios::binary);
  if (!input)
  {
    std::cerr << "Error: Unable to open input file: " << inputFile << std::endl;
    return;
  }
  // std::cout << "1" << std::endl;
  std::string binaryString;
  char byte;
  // std::cout << "2" << std::endl;
  while (input.read(&byte, sizeof(byte)))
  {
    std::bitset<8> binary(byte);
    binaryString += binary.to_string();
  }
  // std::cout << "3" << std::endl;

  server.setItemFromBin(binaryString);
  // std::cout << binaryString << std::endl;
  // std::cout << "4" << std::endl;

  std::cout << "Binary data successfully set in the server item." << std::endl;

  input.close();
}