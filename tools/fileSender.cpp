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

// Ini dipake buat dapetin string dari file yang dikasih, trus disimpan di attribut item di server
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

// MUNGKIN GAKEPAKE
void convertToBinaryAndSetItem(const std::string &inputFile, Server &server)
{
  std::ifstream input(inputFile, std::ios::binary);
  if (!input)
  {
    std::cerr << "Error: Unable to open input file: " << inputFile << std::endl;
    return;
  }
  std::string binaryString;
  char byte;
  while (input.read(&byte, sizeof(byte)))
  {
    std::bitset<8> binary(byte);
    binaryString += binary.to_string();
  }

  server.setItemFromBin(binaryString);

  std::cout << "Binary data successfully set in the server item." << std::endl;

  input.close();
}