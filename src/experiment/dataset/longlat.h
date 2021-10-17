/**
 * @file longlat.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_DATASET_LONGLAT_H_
#define SRC_EXPERIMENT_DATASET_LONGLAT_H_

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "./base_dataset.h"
class LonglatDataset : public BaseDataset {
 public:
  explicit LonglatDataset(float initRatio) : BaseDataset(initRatio) {}

  void GenerateDataset(DataVecType *initDataset, DataVecType *testInsertQuery) {
    DataVecType ds;
    std::ifstream inFile("../experiment/dataset/longlat.csv", std::ios::in);
    if (!inFile) {
      std::cout << "open longlat.csv failed" << std::endl;
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
      if (ds.size() == kDatasetSize + round(kTestSize * (1 - proportion))) {
        break;
      }
    }

    SplitInitTest(&ds, initDataset, testInsertQuery);
  }
};

#endif  // SRC_EXPERIMENT_DATASET_LONGLAT_H_
