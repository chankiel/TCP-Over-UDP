#ifndef tools_h
#define tools_h

#include <iostream>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <bitset>

const std::string INPUT = "[?]";
const std::string IN = "[i]";
const std::string OUT = "[+]";
const std::string WAIT = "[~]";
const std::string ERROR = "[X]";

bool isNumber(const std::string &str);

void commandLine(char symbol, std::string str);
std::string brackets(std::string str);

int generateRandomNumber(int min, int max);
std::string binaryToString(const std::string &binary);
std::string stringToBinary(const std::string &input);
#endif