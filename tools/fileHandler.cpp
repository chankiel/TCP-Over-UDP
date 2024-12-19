#include "fileHandler.hpp"

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

void convertFromStrToFile(const std::string &outputFile, const std::string &content)
{
  std::filesystem::path filePath(outputFile);
  std::filesystem::path dirPath = filePath.parent_path();

  if (!dirPath.empty() && !std::filesystem::exists(dirPath))
  {
    if (!std::filesystem::create_directories(dirPath))
    {
      std::cerr << ERROR << " Unable to create directories for path: " << dirPath << std::endl;
      return;
    }
  }

  std::ofstream output(outputFile);
  if (!output)
  {
    std::cerr << ERROR << " Unable to open output file: " << outputFile << std::endl;
    return;
  }

  output << content;

  std::cout << "OUT: Client content successfully written to the file: " << outputFile << std::endl;

  output.close();
}