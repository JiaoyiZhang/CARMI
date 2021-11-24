/**
 * @file zipfian.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_WORKLOAD_ZIPFIAN_H_
#define SRC_EXPERIMENT_WORKLOAD_ZIPFIAN_H_

#include <math.h>
#include <stdlib.h>

#include <iostream>
#include <random>
class Zipfian {
 public:
  double *pf;
  void InitZipfian(double A, int num) {
    pf = new double[num];
    double sum = 0.0;
    for (int i = 0; i < num; i++) {
      sum += 1 / pow(static_cast<double>(i + 2), A);
    }
    for (int i = 0; i < num; i++) {
      if (i == 0)
        pf[i] = 1 / pow(static_cast<double>(i + 2), A) / sum;
      else
        pf[i] = pf[i - 1] + 1 / pow(static_cast<double>(i + 2), A) / sum;
    }
  }

  int GenerateNextIndex() {
    int index = 0;
    std::default_random_engine e(time(0));
    std::uniform_real_distribution<double> dis(0, 1);
    double data = dis(e);  // 0-1
    while (data > pf[index]) index++;
    return index;
  }
};

#endif  // SRC_EXPERIMENT_WORKLOAD_ZIPFIAN_H_
