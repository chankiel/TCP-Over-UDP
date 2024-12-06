#include <iostream>
#include <fstream>
#include <bitset>
#include <sstream>

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
