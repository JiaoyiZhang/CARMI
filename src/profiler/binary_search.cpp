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

const int kSize = 100000000;
const float kSecondToNanosecond = 1000000000.0;
std::vector<double> entireData;
std::vector<int> idx;
const int end = kSize / 64 - 8;

__always_inline int BinarySearch(double key, int start, int end) {
  while (start < end) {
    int mid = (start + end) / 2;
    if (entireData[mid] < key)
      start = mid + 1;
    else
      end = mid;
  }
  return start;
}

void GetBinarySearchTime(int nodeSize) {
  unsigned seed = std::clock();
  std::default_random_engine engine(seed);
  shuffle(idx.begin(), idx.end(), engine);

  int start, endidx;
  double value;
  std::clock_t s, e;
  double tmp;
  int c;
  s = std::clock();
  for (int i = 0, j = 0; i < end; i++, j++) {
    start = idx[i];
    // endidx = std::min(start + nodeSize - 1, kSize - 1);
    endidx = start + nodeSize - 1;

    value = start + j;
    c = BinarySearch(value, start, endidx);

    j &= nodeSize - 1;
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  s = std::clock();
  for (int i = 0, j = 0; i < end; i++, j++) {
    start = idx[i];
    // endidx = std::min(start + nodeSize - 1, kSize - 1);
    endidx = start + nodeSize - 1;
    value = start + j;
    j &= nodeSize - 1;
  }
  e = std::clock();
  double tmp1 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << nodeSize * 8
            << " bs average time:" << (tmp - tmp1) * kSecondToNanosecond / end
            << std::endl;
}

int main() {
  entireData = std::vector<double>(kSize, 0);
  idx = std::vector<int>(end);
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
