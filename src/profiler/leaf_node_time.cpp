/**
 * @file leaf_node_time.cpp
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-09-18
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <algorithm>
#include <ctime>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "../include/carmi.h"
#include "../include/construct/minor_function.h"
#include "../include/nodes/leafNode/cfarray_type.h"

typedef CARMIRoot<std::vector<std::pair<double, double>>, double> RootType;
typedef CFArrayType<double, double> CFType;
typedef ExternalArray<double, double> ExtType;

const int kSize = 110;
const float kSecondToNanosecond = 1000000000.0;
int block = 512;
int kModelNumber = 100000000;
int end = kModelNumber / block;
int kExternalDatasize = 1024 * end;
std::vector<std::pair<double, double>> dataset(kSize);
std::vector<int> idx(end);

double GetCFNodePredictTime() {
  std::vector<CFType> node(kModelNumber);
  CARMI<double, double> carmi;
  carmi.data = DataArrayStructure<double, double>(
      CFArrayType<double, double>::kMaxBlockNum, kModelNumber / kSize);
  RootType root;
  DataArrayStructure<double, double> data;
  double tmpKeys[CFArrayType<double, double>::kMaxBlockNum - 1];
  for (int i = 0; i < CFArrayType<double, double>::kMaxBlockNum - 1; i++) {
    tmpKeys[i] = dataset[(i + 1) * 16].first;
  }
  for (int i = 0; i < kModelNumber; i++) {
    node[i].flagNumber = 7;
    for (int j = 0; j < CFArrayType<double, double>::kMaxBlockNum - 1; j++) {
      node[i].slotkeys[j] = tmpKeys[j];
    }
  }
  std::vector<int> keys(kSize);
  for (int i = 0; i < kSize; i++) {
    keys[i] = i;
  }

  std::default_random_engine engine(std::clock());
  shuffle(idx.begin(), idx.end(), engine);
  shuffle(keys.begin(), keys.end(), engine);

  int tmpIdx, key;
  int find_idx = 0;
  std::clock_t s, e;
  double tmp, tmp1 = 0;
  std::uniform_int_distribution<int> dis_idx(0, end);
  std::uniform_int_distribution<int> dis_key(0, kSize);
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[dis_idx(engine)];
    key = keys[dis_key(engine)];
    find_idx += node[tmpIdx].Search(key);
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "size:" << sizeof(node[0]) << "\n";
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[dis_idx(engine)];
    key = keys[dis_key(engine)];
    find_idx += node[tmpIdx].m_left + node[tmpIdx].slotkeys[3];
  }
  e = std::clock();
  tmp1 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "find idx:" << find_idx << std::endl;
  return (tmp - tmp1) * kSecondToNanosecond / end;
}

double GetBlockSearchTime() {
  CARMI<double, double> carmi;
  LeafSlots<double, double> tmpSlots;
  for (int i = 0; i < CFArrayType<double, double>::kMaxBlockCapacity; i++) {
    tmpSlots.slots[i] = {i, i * 10};
  }
  DataArrayStructure<double, double> data;
  std::default_random_engine engine(std::clock());
  CFType tmpNode;
  for (int i = 0; i < kModelNumber; i++) {
    data.dataArray.push_back(tmpSlots);
  }
  std::vector<int> keys(CFArrayType<double, double>::kMaxBlockCapacity);
  for (int i = 0; i < CFArrayType<double, double>::kMaxBlockCapacity; i++) {
    keys[i] = i;
  }

  shuffle(idx.begin(), idx.end(), engine);
  shuffle(keys.begin(), keys.end(), engine);

  int tmpIdx, key;
  int find_idx = 0;
  std::clock_t s, e;
  double tmp, tmp1 = 0;
  std::uniform_int_distribution<int> dis_idx(0, end);
  std::uniform_int_distribution<int> dis_key(
      0, CFArrayType<double, double>::kMaxBlockCapacity);
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[dis_idx(engine)];
    key = keys[dis_key(engine)];
    find_idx +=
        tmpNode.SearchDataBlock(data.dataArray[tmpIdx], key,
                                CFArrayType<double, double>::kMaxBlockCapacity);
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[dis_idx(engine)];
    key = keys[dis_key(engine)];
    find_idx += data.dataArray[tmpIdx].slots[0].first +
                data.dataArray[tmpIdx].slots[4].first +
                data.dataArray[tmpIdx].slots[8].first +
                data.dataArray[tmpIdx].slots[12].first;
  }
  e = std::clock();
  tmp1 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "find idx:" << find_idx << std::endl;
  return (tmp - tmp1) * kSecondToNanosecond / end;
}

double GetExternalNodePredictTime() {
  block /= 2;
  kModelNumber /= 100;
  end = kModelNumber / block;
  kExternalDatasize = block * end;
  std::vector<ExtType> node(kModelNumber);
  std::vector<double> keys(block);
  std::default_random_engine engine(std::clock());
  std::uniform_int_distribution<int> dis(0, 4096);
  for (int i = 0; i < block; i++) {
    keys[i] = dis(engine);
  }
  sort(keys.begin(), keys.end());
  std::vector<double> tmpKeys(kExternalDatasize);
  for (int i = 0, j = 0; i < end; i++, j++) {
    j %= block;
    tmpKeys[i] = keys[j];
  }
  std::vector<std::pair<double, double>> initDataset(kExternalDatasize, {1, 1});
  const int record_size = sizeof(double) * 2;
  int extLen = kExternalDatasize * 2 + 10;
  double* externalDataset = new double[extLen];
  for (int i = 0, j = 0; i < kExternalDatasize; i++) {
    *(externalDataset + j) = tmpKeys[i];
    *(externalDataset + j + 1) = tmpKeys[i] * 2;
    j += 2;  // due to <double, double>
    initDataset[i] = {tmpKeys[i], tmpKeys[i] * 2};
  }

  node[0].flagNumber = block;
  node[0].m_left = 0;
  node[0].Train(initDataset, 0, block);
  for (int i = 0; i < kModelNumber; i++) {
    node[i] = node[0];
  }
  idx = std::vector<int>(end);
  for (int i = 0; i < end; i++) {
    idx[i] = i * block;
    node[idx[i]].m_left = i * block;
  }

  shuffle(idx.begin(), idx.end(), engine);
  shuffle(keys.begin(), keys.end(), engine);

  int tmpIdx, key;
  int find_idx = 0;
  std::clock_t s, e;
  double tmp, tmp1 = 0;
  std::uniform_int_distribution<int> dis_idx(0, end);
  std::uniform_int_distribution<int> dis_key(0, block);
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[dis_idx(engine)];
    key = keys[dis_key(engine)];
    find_idx += node[tmpIdx].Find(key, 16, externalDataset);
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "size:" << sizeof(node[0]) << "\n";
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[dis_idx(engine)];
    key = keys[dis_key(engine)];
    find_idx += node[tmpIdx].m_left + node[tmpIdx].error;
  }
  e = std::clock();
  tmp1 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "find idx:" << find_idx << std::endl;
  return (tmp - tmp1) * kSecondToNanosecond / end;
}

int main() {
  for (int i = 0; i < kSize; i++) {
    dataset[i] = {i, i * 10};
  }
  for (int i = 0; i < end; i++) {
    idx[i] = i * block;
  }
  double cf = 0, block = 0, ext = 0;
  float times = 1.0;
  for (int i = 0; i < times; i++) {
    cf += GetCFNodePredictTime();
    block += GetBlockSearchTime();
    ext += GetExternalNodePredictTime();
  }

  std::cout << "cf average time:" << cf / times << std::endl;
  std::cout << "block average time:" << block / times << std::endl;
  std::cout << "external average time:" << ext / times << std::endl;
}
