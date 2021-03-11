/**
 * @file histogram.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_HISTOGRAM_H_
#define SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_HISTOGRAM_H_

#include <algorithm>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

class HistogramModel {
 public:
  HistogramModel() {}
  explicit HistogramModel(int childNum) {
    childNumber = childNum * 2;  // means the max idx of table
    value = 1;
    minValue = 0;
  }
  void Train(const std::vector<std::pair<double, double>> &dataset, int len);
  int Predict(double key) const {
    // return the idx in children
    int idx = static_cast<float>(key - minValue) / value;
    if (idx < 0)
      idx = 0;
    else if (idx >= childNumber)
      idx = childNumber - 1;

    int base;
    int tmpIdx = idx / 16;
    base = Base[tmpIdx];
    int j = idx % 16;
    int tmp = Offset[tmpIdx] >> (15 - j);
    for (; j >= 0; j--) {
      base += tmp & 1;
      tmp = tmp >> 1;
    }
    return base;
  }

 private:
  float value;                         // 4B
  std::vector<unsigned short> Offset;  // 2c/32*8 = c/2 Byte
  std::vector<unsigned short> Base;    // c/2 Byte
  int childNumber;                     // 4B
  float minValue;                      // 8B
};

void HistogramModel::Train(
    const std::vector<std::pair<double, double>> &dataset, int len) {
  if (dataset.size() == 0) return;
  double maxValue;
  for (int i = 0; i < dataset.size(); i++) {
    if (dataset[i].first != -1) {
      minValue = dataset[i].first;
      break;
    }
  }
  for (int i = dataset.size() - 1; i >= 0; i--) {
    if (dataset[i].first != -1) {
      maxValue = dataset[i].first;
      break;
    }
  }
  value = static_cast<float>(maxValue - minValue) / childNumber;
  std::vector<float> table(childNumber, 0);
  int cnt = 0;
  while (cnt <= 1) {
    if (cnt == 1) value /= 10;
    cnt = 0;
    table = std::vector<float>(childNumber, 0);
    for (int i = 0; i < dataset.size(); i++) {
      if (dataset[i].first != -1) {
        int idx = static_cast<float>(dataset[i].first - minValue) / value;
        idx = std::min(idx, static_cast<int>(table.size()) - 1);
        table[idx]++;
      }
    }
    if (table[0] > 0) cnt++;
    table[0] = table[0] / dataset.size() * childNumber;
    for (int i = 1; i < table.size(); i++) {
      if (table[i] > 0) cnt++;
      table[i] = table[i] / dataset.size() * childNumber + table[i - 1];
    }
  }
  table[0] = round(table[0]);
  for (int i = 1; i < childNumber; i++) {
    table[i] = round(table[i]);
    if (table[i] - table[i - 1] > 1) table[i] = table[i - 1] + 1;
  }

  for (int i = 0; i < childNumber; i += 16) {
    unsigned short start_idx = static_cast<int>(table[i]);
    unsigned short tmp = 0;
    for (int j = i; j < i + 16; j++) {
      if (j >= childNumber) {
        while (j < i + 16) {
          tmp = tmp << 1;
          j++;
        }
        Base.push_back(table[i]);
        Offset.push_back(tmp);
        return;
      }
      unsigned short diff = static_cast<int>(table[j]) - start_idx;
      tmp = (tmp << 1) + diff;
#ifdef DEBUG
      if (diff > 1) std::cout << "diff wrong, diff:" << diff << std::endl;
#endif  // DEBUG
      if (diff > 0) start_idx += diff;
    }
    Base.push_back(table[i]);
    Offset.push_back(tmp);
  }
}

#endif  // SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_HISTOGRAM_H_
