/**
 * @file exponential_distribution.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_DATASET_EXPONENTIAL_DISTRIBUTION_H_
#define SRC_EXPERIMENT_DATASET_EXPONENTIAL_DISTRIBUTION_H_

#include <algorithm>

#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "./base_dataset.h"

class ExponentialDataset : public BaseDataset {
 public:
  explicit ExponentialDataset(float initRatio) : BaseDataset(initRatio) {}

  void GenerateDataset(DataVecType *initDataset, DataVecType *testInsertQuery) {
    // create dataset randomly
    std::default_random_engine generator;
    std::exponential_distribution<double> distribution(0.25);

    SplitInitTest<std::exponential_distribution<double>>(
        distribution, initDataset, testInsertQuery);
  }
};

#endif  // SRC_EXPERIMENT_DATASET_EXPONENTIAL_DISTRIBUTION_H_
