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

#include "../../params.h"

class UniformDataset {
 public:
  explicit UniformDataset(float initRatio) {
    proportion = initRatio;
    if (proportion == kRangeScan) {
      proportion = kReadHeavy;
    }
  }

  void GenerateDataset(DataVectorType *initDataset,
                       DataVectorType *trainFindQuery,
                       DataVectorType *trainInsertQuery,
                       std::vector<int> *trainInsertIndex,
                       DataVectorType *testInsertQuery);

 private:
  float proportion;
};

void UniformDataset::GenerateDataset(DataVectorType *initDataset,
                                     DataVectorType *trainFindQuery,
                                     DataVectorType *trainInsertQuery,
                                     std::vector<int> *trainInsertIndex,
                                     DataVectorType *testInsertQuery) {
  DataVectorType().swap(*initDataset);
  DataVectorType().swap(*trainFindQuery);
  DataVectorType().swap(*trainInsertQuery);
  DataVectorType().swap(*testInsertQuery);
  std::vector<int>().swap(*trainInsertIndex);

  std::vector<double> dataset(kDatasetSize + kTestSize * (1 - proportion), 0);

  for (int i = 0; i < dataset.size(); i++) {
    dataset[i] = i;
  }
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine engine(seed);
  shuffle(dataset.begin(), dataset.end(), engine);

  int i = 0;
  int end = round(kTestSize * (1 - proportion));
  for (; i < end; i++) {
    testInsertQuery->push_back({dataset[i], dataset[i] * 10});
  }
  end = dataset.size();
  for (; i < end; i++) {
    initDataset->push_back({dataset[i], dataset[i] * 10});
  }

  std::sort(initDataset->begin(), initDataset->end(),
            [](std::pair<double, double> p1, std::pair<double, double> p2) {
              return p1.first < p2.first;
            });
  std::sort(testInsertQuery->begin(), testInsertQuery->end(),
            [](std::pair<double, double> p1, std::pair<double, double> p2) {
              return p1.first < p2.first;
            });

  trainFindQuery = initDataset;

  if (proportion != kWritePartial && proportion != kReadOnly) {
    int cnt = round(1.0 / (1.0 - proportion));
    for (int j = cnt - 1; j < kDatasetSize; j += cnt) {
      trainInsertQuery->push_back((*initDataset)[j]);
      trainInsertIndex->push_back(j);
    }
  } else if (proportion == kWritePartial) {
    for (int j = kDatasetSize * 0.6; j < kDatasetSize * 0.9; j += 2) {
      trainInsertQuery->push_back((*initDataset)[j]);
      trainInsertIndex->push_back(j);
    }
  }

  std::cout << "uniform: init size:" << (*initDataset).size()
            << "\tFind size:" << (*trainFindQuery).size()
            << "\ttrain insert size:" << (*trainInsertQuery).size()
            << "\tWrite size:" << (*testInsertQuery).size() << std ::endl;
}

#endif  // SRC_EXPERIMENT_DATASET_UNIFORM_DISTRIBUTION_H_
