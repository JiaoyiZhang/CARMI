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

#include "../../../../params.h"
class HistogramModel {
 public:
  explicit HistogramModel(int childNum)
      : base(ceil(childNum * 2 / 16.0), 0),
        offset(ceil(childNum * 2 / 16.0), 0) {
    childNumber = childNum * 2;  // means the max idx of table
    value = 1;
    minValue = 0;
  }
  HistogramModel(const HistogramModel &his) {
    value = his.value;
    childNumber = his.childNumber;
    minValue = his.minValue;
    offset = his.offset;
    base = his.base;
  }
  void Train(const DataVectorType &dataset, int len);
  int Predict(double key) const {
    // return the idx in children
    int idx = static_cast<float>(key - minValue) / value;
    if (idx < 0)
      idx = 0;
    else if (idx >= childNumber)
      idx = childNumber - 1;

    int index = base[(idx >> 4)];
    int tmp = offset[(idx >> 4)] >> (15 - (idx & 0x0000000F));
    tmp = (tmp & 0x55555555) + ((tmp >> 1) & 0x55555555);
    tmp = (tmp & 0x33333333) + ((tmp >> 2) & 0x33333333);
    tmp = (tmp & 0x0f0f0f0f) + ((tmp >> 4) & 0x0f0f0f0f);
    tmp = (tmp & 0x00ff00ff) + ((tmp >> 8) & 0x00ff00ff);
    index += tmp;
    return index;
  }

 public:
  float value;  // 4B
  std::vector<unsigned int> base;
  std::vector<unsigned short int> offset;
  int childNumber;  // 4B
  float minValue;   // 8B
};

void HistogramModel::Train(const DataVectorType &dataset, int len) {
  int actualChildNumber = childNumber / 2;
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
    table[0] = table[0] / dataset.size() * actualChildNumber;
    for (int i = 1; i < table.size(); i++) {
      if (table[i] > 0) cnt++;
      table[i] = table[i] / dataset.size() * actualChildNumber + table[i - 1];
    }
  }
  table[0] = std::max(0, static_cast<int>(round(table[0]) - 1));
  for (int i = 1; i < childNumber; i++) {
    table[i] = std::max(0, static_cast<int>(round(table[i]) - 1));
    if (table[i] - table[i - 1] > 1) table[i] = table[i - 1] + 1;
  }

  int index = 0;
  for (int i = 0; i < childNumber; i += 16) {
    unsigned short start_idx = static_cast<int>(table[i]);
    unsigned short tmp = 0;
    for (int j = i; j < i + 16; j++) {
      if (j >= childNumber) {
        while (j < i + 16) {
          tmp = tmp << 1;
          j++;
        }
        base[index] = table[i];
        offset[index++] = tmp;
        return;
      }
      unsigned short diff = static_cast<int>(table[j]) - start_idx;
      tmp = (tmp << 1) + diff;
      if (diff > 0) start_idx += diff;
    }
    base[index] = table[i];
    offset[index++] = tmp;
  }
}

#endif  // SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_HISTOGRAM_H_
