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
#include <chrono>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "./base_dataset.h"

class UniformDataset : public BaseDataset {
 public:
  explicit UniformDataset(float initRatio) : BaseDataset(initRatio) {}

  void GenerateDataset(carmi_params::DataVectorType *initDataset,
                       carmi_params::DataVectorType *trainFindQuery,
                       carmi_params::DataVectorType *trainInsertQuery,
                       std::vector<int> *trainInsertIndex,
                       carmi_params::DataVectorType *testInsertQuery);
};

void UniformDataset::GenerateDataset(
    carmi_params::DataVectorType *initDataset,
    carmi_params::DataVectorType *trainFindQuery,
    carmi_params::DataVectorType *trainInsertQuery,
    std::vector<int> *trainInsertIndex,
    carmi_params::DataVectorType *testInsertQuery) {
  carmi_params::DataVectorType dataset(
      carmi_params::kDatasetSize + carmi_params::kTestSize * (1 - proportion),
      {0, 0});

  for (int i = 0; i < dataset.size(); i++) {
    dataset[i] = {i, i};
  }
  SplitInitTest(false, initDataset, trainFindQuery, trainInsertQuery,
                trainInsertIndex, testInsertQuery, &dataset);
}

#endif  // SRC_EXPERIMENT_DATASET_UNIFORM_DISTRIBUTION_H_
