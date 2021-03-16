/**
 * @file zipfian.h
 * @author Jiaoyi
 * @brief 
 * @version 0.1
 * @date 2021-03-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef ZIPFIAN_H
#define ZIPFIAN_H

#include <math.h>
#include <stdlib.h>

#include <chrono>
#include <iostream>
using namespace std;
class Zipfian {
 public:
  double *pf;
  void InitZipfian(double A, int num) {
    pf = new double[num];
    double sum = 0.0;
    for (int i = 0; i < num; i++) {
      sum += 1 / pow((double)(i + 2), A);
    }
    for (int i = 0; i < num; i++) {
      if (i == 0)
        pf[i] = 1 / pow((double)(i + 2), A) / sum;
      else
        pf[i] = pf[i - 1] + 1 / pow((double)(i + 2), A) / sum;
    }
  }

  int GenerateNextIndex() {
    srand(time(0));
    int index = 0;
    double data = (double)rand() / RAND_MAX;  // 0-1
    while (data > pf[index]) index++;
    return index;
  }
};

#endif  // !ZIPFIAN_H