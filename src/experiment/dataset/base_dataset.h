/**
 * @file base_dataset.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-26
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef EXPERIMENT_DATASET_BASE_DATASET_H_
#define EXPERIMENT_DATASET_BASE_DATASET_H_

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
                               DataVecType *insertDataset,
                               DataVecType *testInsertQuery) = 0;
  template <typename DistributionType>
  void SplitInitTest(DistributionType &distribution, DataVecType *initDataset,
                     DataVecType *insertDataset, DataVecType *testInsertQuery) {
    (*initDataset) = std::vector<DataType>(kDatasetSize);
    int end = round(kTestSize * (1 - proportion));
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

    std::sort(initDataset->begin(), initDataset->end());
    // generate insertQuery
    if (testInsertQuery->size() > 0) {
      for (int i = 10; i < kDatasetSize - 1; i += 10) {
        double tmp =
            ((*initDataset)[i].first + (*initDataset)[i + 1].first) / 2;
        (*insertDataset).push_back({tmp, tmp * 10});
      }
    }
    std::sort(insertDataset->begin(), insertDataset->end());

    std::cout << "generate dataset over! init size:" << initDataset->size()
              << "\tWrite size:" << testInsertQuery->size() << std::endl;
  }

  void SplitInitTest(DataVecType *dataset, DataVecType *initDataset,
                     DataVecType *insertDataset, DataVecType *testInsertQuery) {
    (*initDataset) = std::vector<DataType>(kDatasetSize);
    int end = round(kTestSize * (1 - proportion));
    (*testInsertQuery) = std::vector<DataType>(end);

    unsigned seed = std::clock();
    std::default_random_engine engine(seed);
    shuffle((*dataset).begin(), (*dataset).end(), engine);

    int i = 0;
    for (int j = 0; i < end; i++, j++) {
      (*testInsertQuery)[j] = (*dataset)[i];
    }
    end = (*dataset).size();
    for (int j = 0; i < end; i++, j++) {
      (*initDataset)[j] = (*dataset)[i];
    }

    std::sort(initDataset->begin(), initDataset->end());
    if (testInsertQuery->size() > 0) {
      for (int i = 10; i < kDatasetSize - 1; i += 10) {
        double tmp =
            ((*initDataset)[i].first + (*initDataset)[i + 1].first) / 2;
        (*insertDataset).push_back({tmp, tmp * 10});
      }
    }
    std::sort(insertDataset->begin(), insertDataset->end());

    std::cout << " init size:" << (*initDataset).size()
              << "\tWrite size:" << (*testInsertQuery).size() << std::endl;
  }
};

#endif  // EXPERIMENT_DATASET_BASE_DATASET_H_
