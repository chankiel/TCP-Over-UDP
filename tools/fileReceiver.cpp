#include <iostream>
#include <fstream>
#include <bitset>
#include <sstream>
#include "../Node/client.hpp"
#include "../Node/server.hpp"

void convertFromBinary(const std::string &binaryFile, const std::string &outputFile)
{
  std::ifstream inFile(binaryFile);
  if (!inFile)
  {
    std::cerr << "Error: Cannot open binary file: " << binaryFile << std::endl;
    return;
  }

  std::ofstream outFile(outputFile, std::ios::binary);
  if (!outFile)
  {
    std::cerr << "Error: Cannot create output file: " << outputFile << std::endl;
    return;
  }

  std::string binaryString;
  while (inFile >> binaryString) // Read each line of binary data
  {
    for (size_t i = 0; i < binaryString.size(); i += 8)
    {
      // Get 8 bits at a time (1 byte)
      std::string byteStr = binaryString.substr(i, 8);

      // Convert the binary string to a byte (char)
      char byte = static_cast<char>(std::bitset<8>(byteStr).to_ulong());

      // Write the byte to the output file
      outFile.write(&byte, sizeof(byte));
    }
  }

  std::cout << "File successfully converted from binary to: " << outputFile << std::endl;

  // Close the files
  inFile.close();
  outFile.close();
}

// Ini dipake stelah client dapat stringnya yg disimpan di atribut item
void convertFromClientToFile(const std::string &outputFile, Client &client)
{
  std::ofstream output(outputFile);
  if (!output)
  {
    std::cerr << "Error: Unable to open output file: " << outputFile << std::endl;
    return;
  }

  std::string clientItem = client.getItem();

  output << clientItem;

  std::cout << "Client content successfully written to the file: " << outputFile << std::endl;

  output.close();
}

// FOR TESTING PURPOSES
void convertFromServerToFile(const std::string &outputFile, Server &server)
{
  std::ofstream output(outputFile);
  if (!output)
  {
    std::cerr << "Error: Unable to open output file: " << outputFile << std::endl;
    return;
  }

  std::string serverItem = server.getItem();

  output << serverItem;

  std::cout << "Server content successfully written to the file: " << outputFile << std::endl;

  output.close();
}
