/**
 * @file example.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-04-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>

#include "./experiment/functions.h"

void TestCarmi() {
  // generate datasets
  int initRatio = carmi_params::kWriteHeavy;
  NormalDataset norData(initRatio);
  carmi_params::TestDataVecType initDataset;
  carmi_params::TestDataVecType testInsert;
  norData.GenerateDataset(&initDataset, &testInsert);
  double rate = 0.1;  // cost = time + rate * space

  CARMICommon<carmi_params::TestKeyType, carmi_params::TestValueType> carmi(
      initDataset, initDataset.size(), initRatio, rate);

  // find the value of the given key
  auto it = carmi.Find(initDataset[0].first);
  std::cout << "res: " << it.data() << std::endl;

  // insert a data point
  carmi_params::TestDataType data = {5, 500};
  auto res = carmi.Insert(data);
  std::cout << " insert is success: " << res << std::endl;

  // update the value of the given key
  res = carmi.Update({initDataset[0].first, 5000});
  std::cout << " Update is success: " << res << std::endl;

  // delete the record of the given key
  res = carmi.Delete(initDataset[0].first);
  std::cout << " Delete is success: " << res << std::endl;
}

void TestExternalCarmi() {
  // generate datasets
  int initRatio = carmi_params::kWriteHeavy;
  NormalDataset norData(initRatio);
  carmi_params::TestDataVecType initDataset;
  carmi_params::TestDataVecType testInsert;
  norData.GenerateDataset(&initDataset, &testInsert);
  double rate = 0.1;  // cost = time + rate * space

  int record_size =
      sizeof(carmi_params::TestKeyType) + sizeof(carmi_params::TestValueType);
  carmi_params::TestKeyType *externalDataset;
  int extLen =
      initDataset.size() * record_size / sizeof(carmi_params::TestKeyType) +
      carmi_params::kReservedSpace;
  externalDataset = new carmi_params::TestKeyType[extLen];
  for (int i = 0, j = 0; i < initDataset.size(); i++) {
    *(externalDataset + j) = initDataset[i].first;
    *(externalDataset + j + 1) = initDataset[i].second;
    j += 2;  // due to <double, double>
  }
  double maxKey = initDataset[initDataset.size() - 1].first;

  CARMIExternal<carmi_params::TestKeyType> carmi(
      externalDataset, initDataset.size(), extLen, record_size, rate);

  // find the value of the given key
  auto it = carmi.Find(initDataset[0].first);
  std::cout << "res: " << it.data()[0] << std::endl;

  // insert a data point
  std::pair<carmi_params::TestKeyType, std::vector<carmi_params::TestKeyType>>
      data = {maxKey + 100, std::vector<carmi_params::TestKeyType>(1, 500)};
  auto res = carmi.Insert(data);
  std::cout << " insert is success: " << res << std::endl;

  // update the value of the given key
  res = carmi.Update({initDataset[0].first, 5000});
  std::cout << " Update is success: " << res << std::endl;

  // delete the record of the given key
  res = carmi.Delete(initDataset[0].first);
  std::cout << " Delete is success: " << res << std::endl;
}

int main() {
  TestExternalCarmi();
  TestCarmi();
  return 0;
}