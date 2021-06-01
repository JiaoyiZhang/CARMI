/**
 * @file ycsb.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-22
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_DATASET_YCSB_H_
#define SRC_EXPERIMENT_DATASET_YCSB_H_

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "./base_dataset.h"

class YCSBDataset : public BaseDataset {
 public:
  explicit YCSBDataset(float initRatio) : BaseDataset(initRatio) {}

  void GenerateDataset(DataVecType *initDataset, DataVecType *testInsertQuery) {
    (*initDataset) = std::vector<DataType>(kDatasetSize);
    int end = round(kTestSize * (1 - proportion));
    (*testInsertQuery) = std::vector<DataType>(end);

    DataVecType ds;
    std::ifstream inFile("..//src//experiment//dataset//newycsbdata.csv",
                         std::ios::in);
    if (!inFile) {
      std::cout << "open ycsb.csv failed" << std::endl;
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
      key.erase(0, 4);  // delete "user"
      double k = stod(key);
      double v = k / 10;
      ds.push_back({k, v});
      if (ds.size() == kDatasetSize + round(kTestSize * (1 - proportion))) {
        break;
      }
    }

    std::sort(ds.begin(), ds.end());
    for (int i = 0; i < kDatasetSize; i++) {
      (*initDataset)[i] = ds[i];
    }

    for (int i = 0; i < end; i++) {
      (*testInsertQuery)[i] = ds[i + kDatasetSize];
    }

    std::cout << "YCSB: init size:" << (*initDataset).size()
              << "\tWrite size:" << (*testInsertQuery).size() << std::endl;
  }
};

#endif  // SRC_EXPERIMENT_DATASET_YCSB_H_
