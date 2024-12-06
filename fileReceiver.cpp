#include <fstream>
#include <iostream>
#include <string>

void convertFromBinary(const std::string &binaryFile,
                       const std::string &outputFile) {
  std::ifstream inFile(binaryFile, std::ios::binary);
  if (!inFile) {
    std::cerr << "Error: Cannot open binary file: " << binaryFile << std::endl;
    return;
  }

  std::ofstream outFile(outputFile, std::ios::binary);
  if (!outFile) {
    std::cerr << "Error: Cannot create output file: " << outputFile
              << std::endl;
    return;
  }

  outFile << inFile.rdbuf();

  std::cout << "File successfully converted to: " << outputFile << std::endl;

  // Close files
  inFile.close();
  outFile.close();
}

int main() {
  std::string binaryFile, outputFile, extension;

  std::cout << "Enter binary file: ";
  std::cin >> binaryFile;

  std::cout << "Enter file extension (e.g., .txt, .png, .pdf): ";
  std::cin >> extension;

  outputFile = "output" + extension;

  convertFromBinary(binaryFile, outputFile);

  return 0;
}
