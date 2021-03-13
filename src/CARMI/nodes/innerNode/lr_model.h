/**
 * @file lr_model.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_LR_MODEL_H_
#define SRC_CARMI_NODES_INNERNODE_LR_MODEL_H_

#include <utility>
#include <vector>

#include "../../carmi.h"

inline void CARMI::initLR(const DataVectorType &dataset, LRModel *lr) {
  int childNumber = lr->flagNumber & 0x00FFFFFF;
  lr->childLeft = allocateChildMemory(childNumber);
  if (dataset.size() == 0) return;

  Train(0, dataset.size(), dataset, lr);

  std::vector<DataVectorType> perSubDataset;
  DataVectorType tmp;
  for (int i = 0; i < childNumber; i++) perSubDataset.push_back(tmp);
  for (int i = 0; i < dataset.size(); i++) {
    int p = lr->Predict(dataset[i].first);
    perSubDataset[p].push_back(dataset[i]);
  }

  switch (kLeafNodeID) {
    case ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        ArrayType tmp(kThreshold);
        initArray(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i],
                  &tmp);
        entireChild[lr->childLeft + i].array = tmp;
      }
      break;
    case GAPPED_ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        GappedArrayType tmp(kThreshold);
        initGA(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i], &tmp);
        entireChild[lr->childLeft + i].ga = tmp;
      }
      break;
  }
}

inline void CARMI::Train(const int left, const int size,
                         const DataVectorType &dataset, LRModel *lr) {
  if (size == 0) return;
  int childNumber = lr->flagNumber & 0x00FFFFFF;
  int end = left + size;
  double maxValue;
  for (int i = left; i < end; i++) {
    lr->minValue = dataset[i].first;
    break;
  }
  for (int i = end - 1; i >= left; i--) {
    maxValue = dataset[i].first;
    break;
  }
  lr->divisor = static_cast<float>(maxValue - lr->minValue) / 6;

  std::vector<double> index;
  int j = 0;
  int cnt = 0;
  for (int i = left; i < end; i++) {
    int idx = static_cast<float>(dataset[i].first - lr->minValue) / lr->divisor;
    if (idx < 0)
      idx = 0;
    else if (idx >= 6)
      idx = 5;
    if (idx != cnt) {
      cnt = idx;
      j = 0;
    }
    index.push_back(static_cast<float>(j));
    j++;
  }

  int i = left;
  cnt = 0;
  for (int k = 1; k <= 6; k++) {
    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (; i < end - 1; i++) {
      if (static_cast<float>(dataset[i].first - lr->minValue) / lr->divisor >=
          k)
        break;
      cnt++;
      t1 += dataset[i].first * dataset[i].first;
      t2 += dataset[i].first;
      t3 += dataset[i].first * index[i - left];
      t4 += index[i - left];
    }
    t3 /= cnt;
    t4 /= cnt;
    if (t1 * cnt - t2 * t2 != 0) {
      auto theta1 = (t3 * cnt - t2 * t4) / (t1 * cnt - t2 * t2);
      auto theta2 = (t1 * t4 - t2 * t3) / (t1 * cnt - t2 * t2);
      theta1 *= childNumber;
      theta2 *= childNumber;
      lr->theta[k - 1] = {theta1, theta2};
    } else {
      lr->theta[k - 1] = {childNumber, 0};
    }
  }
}

inline void CARMI::Train(const int left, const int size, LRModel *lr) {
  if (size == 0) return;
  int childNumber = lr->flagNumber & 0x00FFFFFF;
  int end = left + size;
  double maxValue;
  for (int i = left; i < end; i++) {
    lr->minValue = initDataset[i].first;
    break;
  }
  for (int i = end - 1; i >= left; i--) {
    maxValue = initDataset[i].first;
    break;
  }
  lr->divisor = static_cast<float>(maxValue - lr->minValue) / 6;

  std::vector<double> index;
  int j = 0;
  int cnt = 0;
  for (int i = left; i < end; i++) {
    int idx =
        static_cast<float>(initDataset[i].first - lr->minValue) / lr->divisor;
    if (idx < 0)
      idx = 0;
    else if (idx >= 6)
      idx = 5;
    if (idx != cnt) {
      cnt = idx;
      j = 0;
    }
    index.push_back(static_cast<float>(j));
    j++;
  }

  int i = left;
  cnt = 0;
  for (int k = 1; k <= 6; k++) {
    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (; i < end - 1; i++) {
      if (static_cast<float>(initDataset[i].first - lr->minValue) /
              lr->divisor >=
          k)
        break;
      cnt++;
      t1 += initDataset[i].first * initDataset[i].first;
      t2 += initDataset[i].first;
      t3 += initDataset[i].first * index[i - left];
      t4 += index[i - left];
    }
    t3 /= cnt;
    t4 /= cnt;
    if (t1 * cnt - t2 * t2 != 0) {
      auto theta1 = (t3 * cnt - t2 * t4) / (t1 * cnt - t2 * t2);
      auto theta2 = (t1 * t4 - t2 * t3) / (t1 * cnt - t2 * t2);
      theta1 *= childNumber;
      theta2 *= childNumber;
      lr->theta[k - 1] = {theta1, theta2};
    } else {
      lr->theta[k - 1] = {childNumber, 0};
    }
  }
}

inline int LRModel::Predict(double key) const {
  int idx = static_cast<float>(key - minValue) / divisor;
  if (idx < 0)
    idx = 0;
  else if (idx >= 6)
    idx = 5;
  // return the predicted idx in the children
  int p = theta[idx].first * key + theta[idx].second;
  int bound = (flagNumber & 0x00FFFFFF) / 6;
  int left = bound * idx;
  if (p < left)
    p = left;
  else if (p >= left + bound)
    p = left + bound - 1;
  if (p >= (flagNumber & 0x00FFFFFF)) {
    p = (flagNumber & 0x00FFFFFF) - 1;
  }
  return p;
}
#endif  // SRC_CARMI_NODES_INNERNODE_LR_MODEL_H_
