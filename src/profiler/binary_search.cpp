/**
 * @file binary_search.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-05-24
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

const int kSize = 1410065408;
const float kSecondToNanosecond = 1000000000.0;
std::vector<double> entireData(kSize, 0);
const int end = kSize / 64;
std::vector<int> idx(end, 0);
std::vector<int> start_idx(end, 0);
std::vector<double> value(end, 0.0);
std::vector<int> end_idx(end, 0);

__always_inline int BinarySearch(double key, int start, int end) {
  return entireData[start];// + entireData[start + 1];
  while (start < end) {
    int mid = (start + end) >> 1;
    if (entireData[mid] < key)
      start = mid + 1;
    else
      end = mid;
  }
  return start;
}

void GetBinarySearchTime(int nodeSize) {
  std::vector<int> bias(nodeSize, 0);
  for (int i = 0; i < nodeSize; i++) {
    bias[i] = i;
  }
  unsigned seed = std::clock();
  std::default_random_engine engine(seed);
  shuffle(bias.begin(), bias.end(), engine);
  for (int i = 0; i < end; i++) {
    value[i] = i + bias[i & (nodeSize - 1)];
    start_idx[i] = i;
    end_idx[i] = i + nodeSize - 1;
    // std::cout << "idx[i]:" << idx[i] << ",\tstart_idx[i]:" << start_idx[i]
    //           << ",\tend:" << end_idx[i] << ",\tkey:" << value[i]
    //           << ",\tbias:" << value[i] - start_idx[i] << std::endl;
  }

  shuffle(idx.begin(), idx.end(), engine);

  std::clock_t s, e;
  register int index, startidx, endidx;
  register double key;
  double tmp;
  register int c;
  s = std::clock();
  for (int i = 0; i < end; i++) {
    index = idx[i];
    // key = value[index];
    // startidx = start_idx[index];
    // endidx = end_idx[index];
    c += BinarySearch(index, index, index);
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  double tmp1 = 0;
  shuffle(idx.begin(), idx.end(), engine);
  s = std::clock();
  for (int i = 0; i < end; i++) {
    index = idx[i];
    // c += index;
    // key = value[index];
    // startidx = start_idx[index];
    // endidx = end_idx[index];
    // c += key + startidx + endidx;
  }
  e = std::clock();
  tmp1 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << nodeSize * 8
            << " bs average time:" << (tmp - tmp1) * kSecondToNanosecond / end
            << ",\tc:" << c << std::endl;
}

int main() {
  for (int i = 0; i < kSize; i++) {
    entireData[i] = i;
  }
  for (int i = 0; i < end; i++) {
    idx[i] = i * 64;
  }
  GetBinarySearchTime(64 / 8);
  GetBinarySearchTime(128 / 8);
  GetBinarySearchTime(256 / 8);
  GetBinarySearchTime(512 / 8);
}
