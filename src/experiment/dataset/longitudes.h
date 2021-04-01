/**
 * @file longitudes.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_DATASET_LONGITUDES_H_
#define SRC_EXPERIMENT_DATASET_LONGITUDES_H_

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

#include "./base_dataset.h"
class LongitudesDataset : public BaseDataset {
 public:
  explicit LongitudesDataset(float initRatio) : BaseDataset(initRatio) {}

  void GenerateDataset(carmi_params::TestDataVecType *initDataset,
                       carmi_params::TestDataVecType *trainFindQuery,
                       carmi_params::TestDataVecType *trainInsertQuery,
                       std::vector<int> *trainInsertIndex,
                       carmi_params::TestDataVecType *testInsertQuery);
};

void LongitudesDataset::GenerateDataset(carmi_params::TestDataVecType *initDataset,
                                        carmi_params::TestDataVecType *trainFindQuery,
                                        carmi_params::TestDataVecType *trainInsertQuery,
                                        std::vector<int> *trainInsertIndex,
                                        carmi_params::TestDataVecType *testInsertQuery) {
  carmi_params::TestDataVecType ds;
  std::ifstream inFile("../src/experiment/dataset/longitude.csv", std::ios::in);
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
  std::cout << "longitude size:" << ds.size() << std::endl;

  ds.erase(ds.begin() + carmi_params::kDatasetSize + round(carmi_params::kTestSize * (1 - proportion)),
           ds.end());
  SplitInitTest(false, initDataset, trainFindQuery, trainInsertQuery,
                trainInsertIndex, testInsertQuery, &ds);
}

#endif  // SRC_EXPERIMENT_DATASET_LONGITUDES_H_
