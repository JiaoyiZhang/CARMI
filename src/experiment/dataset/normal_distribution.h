/**
 * @file normal_distribution.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_DATASET_NORMAL_DISTRIBUTION_H_
#define SRC_EXPERIMENT_DATASET_NORMAL_DISTRIBUTION_H_

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "./base_dataset.h"

class NormalDataset : public BaseDataset {
 public:
  explicit NormalDataset(float initRatio) : BaseDataset(initRatio) {}
  void GenerateDataset(carmi_params::TestDataVecType *initDataset,
                       carmi_params::TestDataVecType *trainFindQuery,
                       carmi_params::TestDataVecType *trainInsertQuery,
                       std::vector<int> *trainInsertIndex,
                       carmi_params::TestDataVecType *testInsertQuery);
};

void NormalDataset::GenerateDataset(
    carmi_params::TestDataVecType *initDataset,
    carmi_params::TestDataVecType *trainFindQuery,
    carmi_params::TestDataVecType *trainInsertQuery,
    std::vector<int> *trainInsertIndex,
    carmi_params::TestDataVecType *testInsertQuery) {
  // create dataset randomly
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0.0, 2.0);
  carmi_params::TestDataVecType dataset(
      carmi_params::kDatasetSize + carmi_params::kTestSize * (1 - proportion),
      {0, 0});

  for (int i = 0; i < dataset.size(); i++) {
    double tmp = distribution(generator);
    dataset[i] = {tmp, tmp};
  }
  SplitInitTest(true, initDataset, trainFindQuery, trainInsertQuery,
                trainInsertIndex, testInsertQuery, &dataset);
}

#endif  // SRC_EXPERIMENT_DATASET_NORMAL_DISTRIBUTION_H_
