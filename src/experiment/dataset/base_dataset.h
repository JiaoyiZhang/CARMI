/**
 * @file base_dataset.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-26
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SRC_EXPERIMENT_DATASET_BASE_DATASET_H_
#define SRC_EXPERIMENT_DATASET_BASE_DATASET_H_

#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "../experiment_params.h"

class BaseDataset {
 public:
  float proportion;

  explicit BaseDataset(float init) { proportion = init; }
  virtual void GenerateDataset(DataVecType *initDataset,
                               DataVecType *testInsertQuery) = 0;
  template <typename DistributionType>
  void SplitInitTest(DistributionType &distribution, DataVecType *initDataset,
                     DataVecType *testInsertQuery) {
    (*initDataset) = std::vector<DataType>(kDatasetSize);
    int end = kTestSize * (1 - proportion);
    (*testInsertQuery) = std::vector<DataType>(end);
    std::default_random_engine generator;

    // generate initDataset
    for (int i = 0; i < kDatasetSize; i++) {
      double tmp = distribution(generator) * kMaxValue;
      (*initDataset)[i] = {tmp, tmp * 10};
    }
    // generate testInsertQuery
    for (int i = 0; i < end; i++) {
      double tmp = distribution(generator) * kMaxValue;
      (*testInsertQuery)[i] = {tmp, tmp * 10};
    }

    std::sort(initDataset->begin(), initDataset->end(),
              [](std::pair<double, double> p1, std::pair<double, double> p2) {
                return p1.first < p2.first;
              });
    std::cout << "generate dataset over! init size:" << initDataset->size()
              << "\tWrite size:" << testInsertQuery->size() << std::endl;
  }

  void SplitInitTest(DataVecType *dataset, DataVecType *initDataset,
                     DataVecType *testInsertQuery) {
    (*initDataset) = std::vector<DataType>(kDatasetSize);
    (*testInsertQuery) = std::vector<DataType>(kTestSize);

    unsigned seed = std::clock();
    std::default_random_engine engine(seed);
    shuffle((*dataset).begin(), (*dataset).end(), engine);

    int i = 0;
    int end = round(kTestSize * (1 - proportion));
    for (; i < end; i++) {
      testInsertQuery->push_back((*dataset)[i]);
    }
    end = (*dataset).size();
    for (; i < end; i++) {
      initDataset->push_back((*dataset)[i]);
    }

    std::sort(initDataset->begin(), initDataset->end(),
              [](std::pair<double, double> p1, std::pair<double, double> p2) {
                return p1.first < p2.first;
              });

    std::cout << " init size:" << (*initDataset).size()
              << "\tWrite size:" << (*testInsertQuery).size() << std::endl;
  }
};

#endif  // SRC_EXPERIMENT_DATASET_BASE_DATASET_H_
