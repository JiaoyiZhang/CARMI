/**
 * @file longlat.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_DATASET_LONGLAT_H_
#define SRC_EXPERIMENT_DATASET_LONGLAT_H_

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../../params.h"
class LonglatDataset {
 public:
  explicit LonglatDataset(float initRatio) {
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

void LonglatDataset::GenerateDataset(DataVectorType *initDataset,
                                     DataVectorType *trainFindQuery,
                                     DataVectorType *trainInsertQuery,
                                     std::vector<int> *trainInsertIndex,
                                     DataVectorType *testInsertQuery) {
  DataVectorType().swap(*initDataset);
  DataVectorType().swap(*trainFindQuery);
  DataVectorType().swap(*trainInsertQuery);
  DataVectorType().swap(*testInsertQuery);
  std::vector<int>().swap(*trainInsertIndex);

  DataVectorType ds;
  std::ifstream inFile("../src/experiment/dataset/longlat.csv", std::ios::in);
  if (!inFile) {
    std::cout << "打开文件失败！" << std::endl;
    exit(1);
  }
  std::string line;
  while (getline(inFile, line)) {
    if (line.empty()) continue;
    std::istringstream sin(line);
    std::vector<std::string> fields;
    std::string field;
    while (getline(sin, field, ',')) fields.push_back(field);
    std::string key = fields[0];
    std::string value = fields[1];
    double k = stod(key);
    double v = stod(value);
    ds.push_back({k, v});
  }

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine engine(seed);
  shuffle(ds.begin(), ds.end(), engine);

  int i = 0;
  int end = round(kTestSize * (1 - proportion));
  for (; i < end; i++) {
    testInsertQuery->push_back(ds[i]);
  }
  end = ds.size();
  for (; i < end; i++) {
    initDataset->push_back(ds[i]);
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

  std::cout << "longlat: init size:" << (*initDataset).size()
            << "\tFind size:" << (*trainFindQuery).size()
            << "\ttrain insert size:" << (*trainInsertQuery).size()
            << "\tWrite size:" << (*testInsertQuery).size() << std::endl;
}

#endif  // SRC_EXPERIMENT_DATASET_LONGLAT_H_
