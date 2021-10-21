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
#include "../include/func/inlineFunction.h"
#include "../include/nodes/leafNode/cfarray_type.h"

typedef CARMIRoot<std::vector<std::pair<double, double>>, double> RootType;

const int kSize = 110;
const float kSecondToNanosecond = 1000000000.0;
const int kModelNumber = 100000000;
const int block = 512;
const int end = kModelNumber / block;
std::vector<std::pair<double, double>> dataset(kSize);
std::vector<int> idx(end);

template <typename TYPE>
double GetNodePredictTime() {
  std::vector<TYPE> node(kModelNumber);
  CARMI<double, double> carmi;
  carmi.data = DataArrayStructure<double, double>(
      CFArrayType<double, double>::kMaxBlockNum, kModelNumber / kSize);
  RootType root;
  DataArrayStructure<double, double> data;
  double tmpKeys[6];
  for (int i = 0; i < 6; i++) {
    tmpKeys[i] = dataset[i * 16].first;
  }
  for (int i = 0; i < kModelNumber; i++) {
    node[i].flagNumber = 7;
    for (int i = 0; i < 6; i++) {
      node[i].slotkeys[i] = tmpKeys[i];
    }
  }
  std::vector<int> keys(kSize);
  for (int i = 0; i < kSize; i++) {
    keys[i] = i;
  }

  unsigned seed = std::clock();
  std::default_random_engine engine(seed);
  shuffle(idx.begin(), idx.end(), engine);
  shuffle(keys.begin(), keys.end(), engine);

  int tmpIdx, type, key;
  int currblock, find_idx;
  std::clock_t s, e;
  double tmp, tmp1 = 0;
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[i];
    key = keys[i % kSize];
    int left = node[tmpIdx].m_left;
    currblock = node[tmpIdx].Search(key);
    find_idx += left + currblock;
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "size:" << sizeof(node[0]) << "\n";
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[i];
    key = keys[i % kSize];
    find_idx += node[tmpIdx].m_left + node[tmpIdx].slotkeys[3];
  }
  e = std::clock();
  tmp1 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "find idx:" << find_idx << std::endl;
  return (tmp - tmp1) * kSecondToNanosecond / end;
}

template <typename TYPE>
double GetBlockSearchTime() {
  CARMI<double, double> carmi;
  LeafSlots<double, double> tmpSlots;
  for (int i = 0; i < CFArrayType<double, double>::kMaxBlockCapacity; i++) {
    tmpSlots.slots[i] = {i, i * 10};
  }
  unsigned seed = std::clock();
  DataArrayStructure<double, double> data;
  std::default_random_engine engine(seed);
  TYPE tmpNode;
  for (int i = 0; i < kModelNumber; i++) {
    data.dataArray.push_back(tmpSlots);
  }
  std::vector<int> keys(CFArrayType<double, double>::kMaxBlockCapacity);
  for (int i = 0; i < CFArrayType<double, double>::kMaxBlockCapacity; i++) {
    keys[i] = i;
  }

  shuffle(idx.begin(), idx.end(), engine);
  shuffle(keys.begin(), keys.end(), engine);

  int tmpIdx, type, key;
  int currblock, find_idx, res;
  std::clock_t s, e;
  double tmp, tmp1 = 0;
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[i];
    key = keys[i % CFArrayType<double, double>::kMaxBlockCapacity];
    res += tmpNode.SearchDataBlock(data.dataArray[tmpIdx], key);
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[i];
    key = keys[i % CFArrayType<double, double>::kMaxBlockCapacity];
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

int main() {
  for (int i = 0; i < kSize; i++) {
    dataset[i] = {i, i * 10};
  }
  for (int i = 0; i < end; i++) {
    idx[i] = i * block;
  }
  double cf = 0, block = 0;
  float times = 1.0;
  for (int i = 0; i < times; i++) {
    cf += GetNodePredictTime<CFArrayType<double, double>>();
    block += GetBlockSearchTime<CFArrayType<double, double>>();
  }

  std::cout << "cf average time:" << cf / times << std::endl;
  std::cout << "block average time:" << block / times << std::endl;
}
