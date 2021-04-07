
/**
 * @file main.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
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
#include "./staticRMI/fixed_experiment.h"

std::ofstream outRes;

int main() {
  outRes.open("res_0322.csv", std::ios::app);

  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTest time: " << tmpTime << std::endl;
  outRes << "\nTest time: " << tmpTime << std::endl;

  mainExperiment(carmi_params::kAlgorithmThre);

  outRes << "----------------------------------------------" << std::endl;

  return 0;
}