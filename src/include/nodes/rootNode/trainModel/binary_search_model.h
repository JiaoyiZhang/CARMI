/**
 * @file binary_search_model.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_NODES_ROOTNODE_TRAINMODEL_BINARY_SEARCH_MODEL_H_
#define SRC_INCLUDE_NODES_ROOTNODE_TRAINMODEL_BINARY_SEARCH_MODEL_H_

#include <algorithm>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "../../../../params.h"

class BinarySearchModel {
 public:
  BinarySearchModel() {}
  explicit BinarySearchModel(int childNum) : index(childNum, 0) {
    childNumber = childNum;
  }
  BinarySearchModel(const BinarySearchModel &node) {
    childNumber = node.childNumber;
    index = node.index;
  }
  void Train(const carmi_params::DataVectorType &dataset, int len);
  int Predict(double key) const {
    int start_idx = 0;
    int end_idx = childNumber - 1;
    int mid;
    while (start_idx < end_idx) {
      mid = (start_idx + end_idx) / 2;
      if (index[mid] < key)
        start_idx = mid + 1;
      else
        end_idx = mid;
    }
    return end_idx;
  }

 private:
  std::vector<double> index;  // 8c
  int childNumber;  // 4
};

void BinarySearchModel::Train(const carmi_params::DataVectorType &dataset, int len) {
  if (dataset.size() == 0) return;
  int tmp = 0;
  childNumber = len;
  float value = static_cast<float>(dataset.size()) / childNumber;
  int cnt = 1;
  for (int i = value * cnt - 1; i < dataset.size(); i = value * (++cnt) - 1) {
    if (dataset[i].first != -1) {
      index[tmp++] = dataset[i].first;
    } else {
      for (int j = i + 1; j < dataset.size(); j++) {
        if (dataset[j].first != -1) {
          index[tmp++] = dataset[j].first;
          break;
        }
      }
    }
  }
}

#endif  // SRC_INCLUDE_NODES_ROOTNODE_TRAINMODEL_BINARY_SEARCH_MODEL_H_
