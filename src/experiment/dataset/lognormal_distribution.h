/**
 * @file lognormal_distribution.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_DATASET_LOGNORMAL_DISTRIBUTION_H_
#define SRC_EXPERIMENT_DATASET_LOGNORMAL_DISTRIBUTION_H_

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "./base_dataset.h"

class LognormalDataset : public BaseDataset {
 public:
  explicit LognormalDataset(float initRatio) : BaseDataset(initRatio) {}

  void GenerateDataset(DataVectorType *initDataset,
                       DataVectorType *trainFindQuery,
                       DataVectorType *trainInsertQuery,
                       std::vector<int> *trainInsertIndex,
                       DataVectorType *testInsertQuery);
};

void LognormalDataset::GenerateDataset(DataVectorType *initDataset,
                                       DataVectorType *trainFindQuery,
                                       DataVectorType *trainInsertQuery,
                                       std::vector<int> *trainInsertIndex,
                                       DataVectorType *testInsertQuery) {
  // create dataset randomly
  std::default_random_engine generator;
  std::lognormal_distribution<double> distribution(0.0, 2.0);
  DataVectorType dataset(kDatasetSize + kTestSize * (1 - proportion), {0, 0});

  for (int i = 0; i < dataset.size(); i++) {
    double tmp = distribution(generator);
    dataset[i] = {tmp, tmp};
  }
  SplitInitTest(true, initDataset, trainFindQuery, trainInsertQuery,
                trainInsertIndex, testInsertQuery, &dataset);
}

#endif  // SRC_EXPERIMENT_DATASET_LOGNORMAL_DISTRIBUTION_H_
