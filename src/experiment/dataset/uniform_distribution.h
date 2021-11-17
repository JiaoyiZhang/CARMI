/**
 * @file uniform_distribution.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
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
#include <set>
#include <utility>
#include <vector>

#include "./base_dataset.h"

class UniformDataset : public BaseDataset {
 public:
  explicit UniformDataset(float initRatio) : BaseDataset(initRatio) {}
  void GenerateDataset(DataVecType *initDataset, DataVecType *insertDataset,
                       DataVecType *testInsertQuery) {
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0, 1.0);

    SplitInitTest<std::uniform_real_distribution<double>>(
        distribution, initDataset, insertDataset, testInsertQuery);
    return;
  }
};

#endif  // SRC_EXPERIMENT_DATASET_UNIFORM_DISTRIBUTION_H_
