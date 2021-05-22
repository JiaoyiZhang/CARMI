/**
 * @file uniform_distribution.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-15
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_DATASET_UNIFORM_DISTRIBUTION_H_
#define SRC_EXPERIMENT_DATASET_UNIFORM_DISTRIBUTION_H_

#include <algorithm>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "./base_dataset.h"

class UniformDataset : public BaseDataset {
 public:
  explicit UniformDataset(float initRatio) : BaseDataset(initRatio) {}
  void GenerateDataset(DataVecType *initDataset, DataVecType *testInsertQuery) {
    (*initDataset) = std::vector<DataType>(kDatasetSize);
    int end = kTestSize * (1 - proportion);
    (*testInsertQuery) = std::vector<DataType>(end);

    // generate initDataset
    for (int i = 0; i < kDatasetSize; i++) {
      (*initDataset)[i] = {i * kMaxValue / kDatasetSize, i * 10};
    }
    // generate testInsertQuery
    srand((unsigned)time(NULL));
    for (int i = 0; i < end; i++) {
      int maxValue = kMaxValue;
      int tmp = (rand() % maxValue);
      (*testInsertQuery)[i] = {tmp, tmp * 10};
    }

    std::sort(initDataset->begin(), initDataset->end(),
              [](std::pair<double, double> p1, std::pair<double, double> p2) {
                return p1.first < p2.first;
              });
    std::cout << "generate dataset over! init size:" << initDataset->size()
              << "\tWrite size:" << testInsertQuery->size() << std::endl;
  }
};

#endif  // SRC_EXPERIMENT_DATASET_UNIFORM_DISTRIBUTION_H_
