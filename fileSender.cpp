#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

void convertToBinary(const std::string &inputFile,
                     const std::string &outputFile) {
  std::ifstream inFile(inputFile, std::ios::binary);
  if (!inFile) {
    std::cerr << "Error: Tidak dapat membuka file input: " << inputFile
              << std::endl;
    return;
  }

  std::ofstream outFile(outputFile, std::ios::binary);
  if (!outFile) {
    std::cerr << "Error: Tidak dapat membuat file output: " << outputFile
              << std::endl;
    return;
  }

  outFile << inFile.rdbuf();

  // testing
  inFile.seekg(1, std::ios::beg);
  char s[12];
  inFile.read(s, 12);
  std::cout << s << std::endl;

  std::cout << "File berhasil dikonversi ke format biner: " << outputFile
            << std::endl;

  inFile.close();
  outFile.close();
}

int main() {
  std::string inputFile, outputFile;

  std::cout << "Masukkan nama file input: ";
  std::cin >> inputFile;
  std::cout << "Masukkan nama file output (biner): ";
  std::cin >> outputFile;

  convertToBinary(inputFile, outputFile);

  return 0;
}
