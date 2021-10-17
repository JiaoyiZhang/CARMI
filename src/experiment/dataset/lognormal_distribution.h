/**
 * @file lognormal_distribution.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_DATASET_LOGNORMAL_DISTRIBUTION_H_
#define SRC_EXPERIMENT_DATASET_LOGNORMAL_DISTRIBUTION_H_

#include <algorithm>

#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "./base_dataset.h"

class LognormalDataset : public BaseDataset {
 public:
  explicit LognormalDataset(float initRatio) : BaseDataset(initRatio) {}

  void GenerateDataset(DataVecType *initDataset, DataVecType *testInsertQuery) {
    // create dataset randomly
    std::default_random_engine generator;
    std::lognormal_distribution<double> distribution(0.0, 1.0);

    SplitInitTest<std::lognormal_distribution<double>>(
        distribution, initDataset, testInsertQuery);
  }
};

#endif  // SRC_EXPERIMENT_DATASET_LOGNORMAL_DISTRIBUTION_H_
