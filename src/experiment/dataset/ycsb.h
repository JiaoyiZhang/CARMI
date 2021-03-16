#ifndef YCSB_H
#define YCSB_H

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "../../params.h"
extern bool kPrimaryIndex;

class YCSBDataset {
 public:
  explicit YCSBDataset(float initRatio) {
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

void YCSBDataset::GenerateDataset(DataVectorType *initDataset,
                                  DataVectorType *trainFindQuery,
                                  DataVectorType *trainInsertQuery,
                                  std::vector<int> *trainInsertIndex,
                                  DataVectorType *testInsertQuery) {
  DataVectorType().swap((*initDataset));
  DataVectorType().swap((*trainFindQuery));
  DataVectorType().swap((*trainInsertQuery));
  DataVectorType().swap((*testInsertQuery));
  std::vector<int>().swap(*trainInsertIndex);

  DataVectorType ds;
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
    if (ds.size() == kDatasetSize + kTestSize * (1 - proportion)) {
      break;
    }
  }

  std::sort(ds.begin(), ds.end());

  int i = 0;
  int end = kDatasetSize;
  for (; i < end; i++) {
    initDataset->push_back(ds[i]);
  }
  end = ds.size();
  for (; i < end; i++) {
    testInsertQuery->push_back(ds[i]);
  }

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

  std::cout << "YCSB: init size:" << (*initDataset).size()
            << "\tFind size:" << (*trainFindQuery).size()
            << "\ttrain insert size:" << (*trainInsertQuery).size()
            << "\tWrite size:" << (*testInsertQuery).size() << std::endl;
}

#endif