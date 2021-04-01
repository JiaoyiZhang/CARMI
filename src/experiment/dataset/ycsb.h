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
#include <chrono>
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

  void GenerateDataset(carmi_params::TestDataVecType *initDataset,
                       carmi_params::TestDataVecType *trainFindQuery,
                       carmi_params::TestDataVecType *trainInsertQuery,
                       std::vector<int> *trainInsertIndex,
                       carmi_params::TestDataVecType *testInsertQuery);
};

void YCSBDataset::GenerateDataset(
    carmi_params::TestDataVecType *initDataset,
    carmi_params::TestDataVecType *trainFindQuery,
    carmi_params::TestDataVecType *trainInsertQuery,
    std::vector<int> *trainInsertIndex,
    carmi_params::TestDataVecType *testInsertQuery) {
  carmi_params::TestDataVecType().swap((*initDataset));
  carmi_params::TestDataVecType().swap((*trainFindQuery));
  carmi_params::TestDataVecType().swap((*trainInsertQuery));
  carmi_params::TestDataVecType().swap((*testInsertQuery));
  std::vector<int>().swap(*trainInsertIndex);

  carmi_params::TestDataVecType ds;
  std::ifstream inFile("..//src//experiment//dataset//newycsbdata.csv",
                       std::ios::in);
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
    key.erase(0, 4);  // delete "user"
    double k = stod(key);
    double v = k / 10;
    ds.push_back({k, v});
    if (ds.size() == carmi_params::kDatasetSize) {
      break;
    }
  }

  std::sort(ds.begin(), ds.end());

  int i = 0;
  int end = carmi_params::kDatasetSize;
  for (; i < end; i++) {
    initDataset->push_back(ds[i]);
  }
  carmi_params::kExternalInsertLeft =
      carmi_params::kDatasetSize - carmi_params::kTestSize * (1 - proportion);
  i = carmi_params::kExternalInsertLeft;
  for (; i < carmi_params::kDatasetSize; i++) {
    testInsertQuery->push_back((*initDataset)[i]);
    trainInsertIndex->push_back(i);
  }

  trainFindQuery->insert(trainFindQuery->begin(), initDataset->begin(),
                         initDataset->end());
  trainInsertQuery->insert(trainInsertQuery->begin(), testInsertQuery->begin(),
                           testInsertQuery->end());

  std::cout << "YCSB: init size:" << (*initDataset).size()
            << "\tFind size:" << (*trainFindQuery).size()
            << "\ttrain insert size:" << (*trainInsertQuery).size()
            << "\tWrite size:" << (*testInsertQuery).size() << std::endl;
}

#endif  // SRC_EXPERIMENT_DATASET_YCSB_H_
