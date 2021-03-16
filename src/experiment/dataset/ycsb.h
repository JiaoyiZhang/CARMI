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
  YCSBDataset(double initRatio) {
    init = initRatio;
    insertNumber = 100000 * (1 - initRatio);
    if (initRatio == 0) {
      num = 0;
      init = 0.85;
      insertNumber = 15000;
    } else if (initRatio == 1)
      num = -1;
    else
      num = round(initRatio / (1 - initRatio));
  }

  void GenerateDataset(DataVectorType *initDataset,
                       DataVectorType *trainFindQuery,
                       DataVectorType *trainInsertQuery,
                       DataVectorType *testInsertQuery);

 private:
  int num;
  float init;
  int insertNumber;
};

void YCSBDataset::GenerateDataset(DataVectorType *initDataset,
                                  DataVectorType *trainFindQuery,
                                  DataVectorType *trainInsertQuery,
                                  DataVectorType *testInsertQuery) {
  DataVectorType().swap((*initDataset));
  DataVectorType().swap((*trainFindQuery));
  DataVectorType().swap((*trainInsertQuery));
  DataVectorType().swap((*testInsertQuery));

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
    if (ds.size() == round(67108864.0 / init)) break;
  }

  std::sort(ds.begin(), ds.end());
  for (int i = 0; i < ds.size(); i++) (*initDataset).push_back(ds[i]);
  int end = round(67108864 / init * (1 - init));
  auto maxValue = ds[ds.size() - 1];
  end = round(100000 * (1 - init));
  for (int i = 1; i <= end; i++)
    (*testInsertQuery).push_back({maxValue.first + i, maxValue.second + i});

  std::default_random_engine engine;

  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
  engine = std::default_random_engine(seed1);
  trainFindQuery = initDataset;
  for (int i = 0; i < (*trainFindQuery).size(); i++) {
    (*trainFindQuery)[i].second = 1;
  }

  std::cout << "YCSB: init size:" << (*initDataset).size()
            << "\tFind size:" << (*trainFindQuery).size()
            << "\ttrain insert size:" << (*trainInsertQuery).size()
            << "\tWrite size:" << (*testInsertQuery).size() << std::endl;
}

#endif