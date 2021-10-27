/**
 * @file inner_node_time.cpp
 * @author Jiaoyi
 * @brief
 * @version 3.0
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

const int kSize = 1024;
const float kSecondToNanosecond = 1000000000.0;
const int kModelNumber = 100000000;
const int block = 512;
const int end = kModelNumber / block;
std::vector<std::pair<double, double>> data(kSize);
std::vector<int> idx(end);

template <typename TYPE>
double GetNodePredictTime() {
  std::vector<TYPE> node(kModelNumber, TYPE(20));
  node[0].Train(0, kSize, data);
  for (int i = 0; i < end; i++) {
    node[i * block] = node[0];
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
  srand(time(0));
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[rand() % end];
    key = keys[rand() % kSize];
    res = node[tmpIdx].Predict(key);
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  s = std::clock();
  for (int i = 0; i < end; i++) {
    tmpIdx = idx[rand() % end];
    key = keys[rand() % kSize];
    res = node[tmpIdx].flagNumber + node[tmpIdx].childLeft;
  }
  e = std::clock();
  tmp1 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  return (tmp - tmp1) * kSecondToNanosecond / end;
}

int main() {
  for (int i = 0; i < kSize; i++) {
    data[i] = {i, i * 10};
  }
  for (int i = 0; i < end; i++) {
    idx[i] = i * block;
  }
  double lr = 0, plr = 0, bs = 0, his = 0;
  float times = 1.0;
  for (int i = 0; i < times; i++) {
    lr += GetNodePredictTime<LRModel<double, double>>();
    plr += GetNodePredictTime<PLRModel<double, double>>();
    his += GetNodePredictTime<HisModel<double, double>>();
    bs += GetNodePredictTime<BSModel<double, double>>();
  }

  std::cout << "lr average time:" << lr / times << std::endl;
  std::cout << "plr average time:" << plr / times << std::endl;
  std::cout << "his average time:" << his / times << std::endl;
  std::cout << "bs average time:" << bs / times << std::endl;
}
