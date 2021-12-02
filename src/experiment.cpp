/**
 * @file main.cpp
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>

#include "./experiment/functions.h"

std::ofstream outRes;

int main() {
  kPrimaryIndex = false;
  outRes.open("res_1122.csv", std::ios::app);

  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTest time: " << tmpTime << std::endl;
  outRes << "\nTest time: " << tmpTime << std::endl;

  mainExperiment();

  outRes << "----------------------------------------------" << std::endl;

  return 0;
}
