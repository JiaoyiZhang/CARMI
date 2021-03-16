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

#include "../../params.h"

class LognormalDataset {
 public:
  explicit LognormalDataset(float initRatio) {
    proportion = initRatio;
    if (proportion == kRangeScan) {
      proportion = kReadHeavy;
    }
  }

  void GenerateDataset(DataVectorType *initDataset,
                       DataVectorType *trainFindQuery,
                       DataVectorType *trainInsertQuery,
                       DataVectorType *testInsertQuery);

 private:
  float proportion;
};

void LognormalDataset::GenerateDataset(DataVectorType *initDataset,
                                       DataVectorType *trainFindQuery,
                                       DataVectorType *trainInsertQuery,
                                       DataVectorType *testInsertQuery) {
  DataVectorType().swap(*initDataset);
  DataVectorType().swap(*trainFindQuery);
  DataVectorType().swap(*trainInsertQuery);
  DataVectorType().swap(*testInsertQuery);

  // create dataset randomly
  std::default_random_engine generator;
  std::lognormal_distribution<double> distribution(0.0, 2.0);
  std::vector<double> dataset(kDatasetSize + kTestSize * (1 - proportion), 0);

  for (int i = 0; i < dataset.size(); i++) {
    dataset[i] = distribution(generator);
  }
  std::sort(dataset.begin(), dataset.end());

  double diff = 0;
  double maxV = dataset[dataset.size() - 1];
  if (dataset[0] < 0) {
    diff = -dataset[0];
    maxV += diff;
  }
  double factor = kMaxValue / maxV;
  for (int j = 0; j < dataset.size(); j++) {
    dataset[j] = (dataset[j] + diff) * factor;
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
    }
  } else if (proportion == kWritePartial) {
    for (int j = kDatasetSize * 0.6; j < kDatasetSize * 0.9; j += 2) {
      trainInsertQuery->push_back((*initDataset)[j]);
    }
  }

  std::cout << "lognormal: init size:" << (*initDataset).size()
            << "\tFind size:" << (*trainFindQuery).size()
            << "\ttrain insert size:" << (*trainInsertQuery).size()
            << "\tWrite size:" << (*testInsertQuery).size() << std::endl;
}

#endif  // SRC_EXPERIMENT_DATASET_LOGNORMAL_DISTRIBUTION_H_
