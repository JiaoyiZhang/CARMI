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
#include <chrono>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "../../params.h"

class BaseDataset {
 public:
  explicit BaseDataset(float init) { proportion = init; }
  virtual void GenerateDataset(DataVectorType *initDataset,
                               DataVectorType *trainFindQuery,
                               DataVectorType *trainInsertQuery,
                               std::vector<int> *trainInsertIndex,
                               DataVectorType *testInsertQuery) = 0;
  void SplitInitTest(bool NeedScale, DataVectorType *initDataset,
                     DataVectorType *trainFindQuery,
                     DataVectorType *trainInsertQuery,
                     std::vector<int> *trainInsertIndex,
                     DataVectorType *testInsertQuery, DataVectorType *dataset);
  void ScaleDataset(DataVectorType *dataset) {
    double diff = 0;
    double maxV = (*dataset)[(*dataset).size() - 1].first;
    if ((*dataset)[0].first < 0) {
      diff = -(*dataset)[0].first;
      maxV += diff;
    }
    double factor = kMaxValue / maxV;
    for (int j = 0; j < (*dataset).size(); j++) {
      (*dataset)[j].first = ((*dataset)[j].first + diff) * factor;
      (*dataset)[j].second = ((*dataset)[j].second + diff) * factor * 10;
    }
  }

  float proportion;
};

void BaseDataset::SplitInitTest(bool NeedScale, DataVectorType *initDataset,
                                DataVectorType *trainFindQuery,
                                DataVectorType *trainInsertQuery,
                                std::vector<int> *trainInsertIndex,
                                DataVectorType *testInsertQuery,
                                DataVectorType *dataset) {
  DataVectorType().swap(*initDataset);
  DataVectorType().swap(*trainFindQuery);
  DataVectorType().swap(*trainInsertQuery);
  DataVectorType().swap(*testInsertQuery);
  std::vector<int>().swap(*trainInsertIndex);

  if (NeedScale) {
    std::sort(dataset->begin(), dataset->end(),
              [](std::pair<double, double> p1, std::pair<double, double> p2) {
                return p1.first < p2.first;
              });
    ScaleDataset(dataset);
  }

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
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

  trainFindQuery->insert(trainFindQuery->begin(), initDataset->begin(),
                         initDataset->end());

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

  std::cout << "exponential: init size:" << (*initDataset).size()
            << "\tFind size:" << (*trainFindQuery).size()
            << "\ttrain insert size:" << (*trainInsertQuery).size()
            << "\tWrite size:" << (*testInsertQuery).size() << std::endl;
}
#endif  // SRC_EXPERIMENT_DATASET_BASE_DATASET_H_
