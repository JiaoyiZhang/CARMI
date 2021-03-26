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
  HistogramModel() {}
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
  double maxValue = dataset[dataset.size() - 1].first;
  minValue = dataset[0].first;
  value = static_cast<float>(maxValue - minValue) / childNumber;

  // count the number of data points in each child
  std::vector<int> table(childNumber, 0);
  for (int i = 0; i < dataset.size(); i++) {
    int idx = (dataset[i].first - minValue) / value;
    idx = std::min(std::max(0, idx), childNumber - 1);
    table[idx]++;
  }

  // normalize table
  std::vector<double> tmpIdx(childNumber, 0);
  tmpIdx[0] =
      static_cast<double>(table[0]) / dataset.size() * (actualChildNumber - 1);
  for (int i = 1; i < childNumber; i++) {
    tmpIdx[i] = static_cast<double>(table[i]) / dataset.size() *
                    (actualChildNumber - 1) +
                tmpIdx[i - 1];
  }

  table[0] = round(tmpIdx[0]);
  for (int i = 1; i < childNumber; i++) {
    table[i] = round(tmpIdx[i]);
    if (table[i] - table[i - 1] > 1) {
      table[i] = table[i - 1] + 1;
    }
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
