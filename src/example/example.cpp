/**
 * @file example.cpp
 * @author Jiaoyi
 * @brief The examples of CARMI
 * @version 3.0
 * @date 2021-04-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>

#include "../include/carmi_external_map.h"
#include "../include/carmi_map.h"
const float kWriteHeavy = 0.5;  // write-heavy workload

void TestCarmi() {
  // generate datasets
  std::vector<std::pair<double, double>> initDataset(10, {1, 1});
  for (int i = 0; i < 10; i++) {
    initDataset[i].first = i * 2;
  }

  CARMIMap<double, double> carmi(initDataset.begin(), initDataset.end());

  // find the value of the given key
  auto it = carmi.find(initDataset[0].first);
  std::cout << "1.  FIND is successful, the value of the given key is: "
            << it.data() << std::endl;
  std::cout << "    Current and all subsequent key-value pairs:";
  for (; it != carmi.end(); ++it) {
    std::cout << "{" << it.key() << ", " << it.data() << "}  ";
  }
  std::cout << std::endl;

  // insert a data point
  std::pair<double, double> data = {5, 500};
  auto res = carmi.insert(data);
  std::cout << "2.  INSERT is successful!" << std::endl;

  it = carmi.find(data.first);
  std::cout
      << "    FIND after INSERT is successful, the value of the given key is: "
      << it.data() << std::endl;
  std::cout << "      Current and all subsequent key-value pairs:";
  for (; it != carmi.end(); ++it) {
    std::cout << "{" << it.key() << ", " << it.data() << "}  ";
  }
  std::cout << std::endl;

  // delete the record of the given key
  int cnt = carmi.erase(initDataset[0].first);
  if (cnt > 0)
    std::cout << "4.  DELETE is successful!" << std::endl;
  else
    std::cout << "  DELETE failed!" << std::endl;
  it = carmi.find(initDataset[0].first);
  if (it == carmi.end() || it.key() != initDataset[0].first) {
    std::cout << "    FIND after DELETE failed." << std::endl;
  }
}

template <typename KeyType, typename ValueType>
class ExternalDataType {
 public:
  typedef ValueType ValueType_;
  ExternalDataType() {
    k = 0;
    v = 0;
  }
  explicit ExternalDataType(KeyType key, ValueType_ value) {
    k = key;
    v = value;
  }
  const KeyType &key() const { return k; }
  const ValueType_ &data() const { return v; }

  bool operator<(const ExternalDataType &a) const {
    if (k == a.k) {
      return v < a.v;
    }
    return k < a.k;
  }

  KeyType k;
  ValueType_ v;
};

void TestExternalCarmi() {
  // generate datasets
  int initRatio = kWriteHeavy;
  int size = 10;
  std::vector<std::pair<double, double>> initDataset(size, {1, 1});
  for (int i = 0; i < size; i++) {
    initDataset[i].first = i * 2;
  }

  const int record_size = sizeof(double) * 2;
  int extLen = initDataset.size() * 2 + 10;
  double *externalDataset = new double[extLen];
  for (int i = 0, j = 0; i < initDataset.size(); i++) {
    *(externalDataset + j) = initDataset[i].first;
    *(externalDataset + j + 1) = initDataset[i].second;
    j += 2;  // due to <double, double>
  }
  double maxKey = initDataset[initDataset.size() - 1].first;
  std::vector<double> futureinsertKey(1, maxKey + 1);

  CARMIExternalMap<double, ExternalDataType<double, double>> carmi(
      externalDataset, futureinsertKey, initDataset.size(), record_size);

  // find the value of the given key
  auto it = carmi.find(initDataset[4].first);
  std::cout << "1.  FIND is successful, the given key is: " << it.key()
            << ",\tthe value is: " << it.data() << std::endl;

  // insert data into the external array
  *(externalDataset + size * 2) = futureinsertKey[0];
  *(externalDataset + size * 2 + 1) = 100;

  // insert a data point
  carmi.insert(futureinsertKey[0]);  // insert key into carmi
  std::cout << "2.  INSERT is successful!" << std::endl;
  it = carmi.find(futureinsertKey[0]);
  std::cout << "      FIND is successful, the given key is: " << it.key()
            << ",\tthe value is: " << it.data() << std::endl;
}

int main() {
  std::cout << "Test carmi:" << std::endl;
  TestCarmi();
  std::cout << "Test external carmi:" << std::endl;
  TestExternalCarmi();
  return 0;
}