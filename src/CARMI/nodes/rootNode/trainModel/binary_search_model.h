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
#ifndef SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_BINARY_SEARCH_MODEL_H_
#define SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_BINARY_SEARCH_MODEL_H_

#include <algorithm>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "../../../../params.h"

class BinarySearchModel {
 public:
  BinarySearchModel() {}
  explicit BinarySearchModel(int childNum) {
    childNumber = childNum;
    index = std::vector<double>(childNum, 0);
  }
  void Train(const DataVectorType &dataset, int len);
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

  //   // designed for test
  //   void GetIndex std::vector<double> &v) {
  //     for (int i = 0; i < childNumber; i++) v.push_back(index[i]);
  //   }
  //   int GetChildNum() { return childNumber; }

 private:
  std::vector<double> index;  // 8c
  // double index[131072]; // 8c
  int childNumber;  // 4
};

void BinarySearchModel::Train(const DataVectorType &dataset, int len) {
  if (dataset.size() == 0) return;
  // index.clear();
  int tmp = 0;
  childNumber = len;
  float value = static_cast<float>(dataset.size()) / childNumber;
  int cnt = 1;
  for (int i = value * cnt - 1; i < dataset.size(); i = value * (++cnt) - 1) {
    if (dataset[i].first != -1) {
      index[tmp++] = dataset[i].first;
      // index.push_back(dataset[i].first);
    } else {
      for (int j = i + 1; j < dataset.size(); j++) {
        if (dataset[j].first != -1) {
          // index.push_back(dataset[j].first);
          index[tmp++] = dataset[j].first;
          break;
        }
      }
    }
  }
}

#endif  // SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_BINARY_SEARCH_MODEL_H_
