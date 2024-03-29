/**
 * @file longitudes.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef EXPERIMENT_DATASET_LONGITUDES_H_
#define EXPERIMENT_DATASET_LONGITUDES_H_

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
class LongitudesDataset : public BaseDataset {
 public:
  explicit LongitudesDataset(float initRatio) : BaseDataset(initRatio) {}

  void GenerateDataset(DataVecType *initDataset, DataVecType *insertDataset,
                       DataVecType *testInsertQuery) {
    DataVecType ds;
    std::ifstream inFile("../experiment/dataset/longitude.csv", std::ios::in);
    if (!inFile) {
      std::cout << "open longitude.csv failed" << std::endl;
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

    SplitInitTest(&ds, initDataset, insertDataset, testInsertQuery);
  }
};

#endif  // EXPERIMENT_DATASET_LONGITUDES_H_
