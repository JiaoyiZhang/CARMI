/**
 * @file inner_node_time.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-05-25
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
#include "../include/nodes/innerNode/bs_model.h"
#include "../include/nodes/innerNode/his_model.h"
#include "../include/nodes/innerNode/lr_model.h"
#include "../include/nodes/innerNode/plr_model.h"

const int kSize = 128;
const float kSecondToNanosecond = 1000000000.0;
const int kModelNumber = 100000000;
const int block = 512;
const int end = kModelNumber / block;
std::vector<std::pair<double, double>> entireData(kSize);
std::vector<int> idx(end);

template <typename TYPE>
double GetNodePredictTime(CARMI<double, double> carmi) {
  std::vector<TYPE> node(kModelNumber);
  for (int i = 0; i < kModelNumber; i++) {
    node[i].SetChildNumber(20);
  }
  for (int i = 0; i < end; i++) {
    carmi.Train(0, kSize, entireData, &node[i * block]);
  }
  std::vector<int> keys(kSize);
  for (int i = 0; i < kSize; i++) {
    keys[i] = i;
  }

  unsigned seed = std::clock();
  std::default_random_engine engine(seed);
  shuffle(idx.begin(), idx.end(), engine);
  shuffle(keys.begin(), keys.end(), engine);

  int start, endidx, tmpIdx, type, key;
  int nodeSize = 8;
  double res;
  std::clock_t s, e;
  double tmp, tmp1 = 0;
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[i];
    key = keys[i % kSize];
    res = node[tmpIdx].childLeft + node[tmpIdx].Predict(key);
    type = node[tmpIdx + 256].flagNumber >> 24;
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[i];
    key = keys[i % kSize];
    // res = node[tmpIdx].childLeft + node[tmpIdx].Predict(key);
    // type = node[res + 256].flagNumber >> 24;
  }
  e = std::clock();
  tmp1 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  return (tmp - tmp1) * kSecondToNanosecond / end;
}

int main() {
  for (int i = 0; i < kSize; i++) {
    entireData[i] = {i, i * 10};
  }
  for (int i = 0; i < end; i++) {
    idx[i] = i * block;
  }
  CARMI<double, double> carmi;
  double lr = 0, plr = 0, bs = 0, his = 0;
  float times = 1.0;
  for (int i = 0; i < times; i++) {
    lr += GetNodePredictTime<LRModel>(carmi);
    plr += GetNodePredictTime<PLRModel>(carmi);
    his += GetNodePredictTime<HisModel>(carmi);
    bs += GetNodePredictTime<BSModel>(carmi);
  }

  std::cout << "lr average time:" << lr / times << std::endl;
  std::cout << "plr average time:" << plr / times << std::endl;
  std::cout << "his average time:" << his / times << std::endl;
  std::cout << "bs average time:" << bs / times << std::endl;
}
