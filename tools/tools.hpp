#ifndef tools_h
#define tools_h

#include <iostream>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>

bool isNumber(const std::string& str);

void commandLine(char symbol, std::string str);

int generateRandomNumber(int min, int max);
#endif