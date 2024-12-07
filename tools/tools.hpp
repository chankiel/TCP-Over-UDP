#ifndef tools_h
#define tools_h

#include <iostream>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>

const std::string INPUT = "[?]";
const std::string IN = "[i]";
const std::string OUT = "[+]";
const std::string WAIT = "[~]";

bool isNumber(const std::string& str);

void commandLine(char symbol, std::string str);
std::string brackets(std::string str);

int generateRandomNumber(int min, int max);
#endif