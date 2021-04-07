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
  virtual void GenerateDataset(
      carmi_params::TestDataVecType *initDataset,
      carmi_params::TestDataVecType *testInsertQuery) = 0;
  void SplitInitTest(bool NeedScale, carmi_params::TestDataVecType *initDataset,
                     carmi_params::TestDataVecType *testInsertQuery,
                     carmi_params::TestDataVecType *dataset) {
    carmi_params::TestDataVecType().swap(*initDataset);
    carmi_params::TestDataVecType().swap(*testInsertQuery);

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
    int end = round(carmi_params::kTestSize * (1 - proportion));
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
  void ScaleDataset(carmi_params::TestDataVecType *dataset) {
    double diff = 0;
    double maxV = (*dataset)[(*dataset).size() - 1].first;
    if ((*dataset)[0].first < 0) {
      diff = -(*dataset)[0].first;
      maxV += diff;
    }
    double factor = carmi_params::kMaxValue / maxV;
    for (int j = 0; j < (*dataset).size(); j++) {
      (*dataset)[j].first = ((*dataset)[j].first + diff) * factor;
      (*dataset)[j].second = ((*dataset)[j].second + diff) * factor * 10;
    }
  }

  float proportion;
};

#endif  // SRC_EXPERIMENT_DATASET_BASE_DATASET_H_
