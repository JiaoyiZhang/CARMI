/**
 * @file bs_model.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_BS_MODEL_H_
#define SRC_CARMI_NODES_INNERNODE_BS_MODEL_H_

#include <algorithm>
#include <utility>
#include <vector>

#include "../../carmi.h"

inline void CARMI::initBS(const DataVectorType &dataset, BSModel *bs) {
  int childNumber = bs->flagNumber & 0x00FFFFFF;
  bs->childLeft = allocateChildMemory(childNumber);
  if (dataset.size() == 0) return;

  Train(0, dataset.size(), dataset, bs);

  std::vector<DataVectorType> perSubDataset;
  DataVectorType tmp;
  for (int i = 0; i < childNumber; i++) perSubDataset.push_back(tmp);
  for (int i = 0; i < dataset.size(); i++) {
    int p = bs->Predict(dataset[i].first);
    perSubDataset[p].push_back(dataset[i]);
  }

  switch (kLeafNodeID) {
    case ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        ArrayType tmp(kThreshold);
        initArray(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i],
                  &tmp);
        entireChild[bs->childLeft + i].array = tmp;
      }
      break;
    case GAPPED_ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        GappedArrayType tmp(kThreshold);
        initGA(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i], &tmp);
        entireChild[bs->childLeft + i].ga = tmp;
      }
      break;
  }
}

inline void CARMI::Train(const int left, const int size,
                         const DataVectorType &dataset, BSModel *bs) {
  if (size == 0) return;
  int childNumber = bs->flagNumber & 0x00FFFFFF;
  float value = static_cast<float>(size) / childNumber;
  int cnt = 1;
  int start = std::min(static_cast<float>(left), left + value * cnt - 1);
  int end = left + size;
  for (int i = start; i < end; i += value) {
    if (cnt >= childNumber) break;
    if (dataset[i].first != -1) {
      bs->index[cnt - 1] = dataset[i].first;
    } else {
      for (int j = i + 1; j < end; j++) {
        if (dataset[j].first != -1) {
          bs->index[cnt - 1] = dataset[i].first;
          break;
        }
      }
    }
    cnt++;
  }
}

inline void CARMI::Train(const int left, const int size, BSModel *bs) {
  if (size == 0) return;
  int childNumber = bs->flagNumber & 0x00FFFFFF;
  float value = static_cast<float>(size) / childNumber;
  int cnt = 1;
  int start = std::min(static_cast<float>(left), left + value * cnt - 1);
  int end = left + size;
  for (int i = start; i < end; i += value) {
    if (cnt >= childNumber) break;
    if (initDataset[i].first != -1) {
      bs->index[cnt - 1] = initDataset[i].first;
    } else {
      for (int j = i + 1; j < end; j++) {
        if (initDataset[j].first != -1) {
          bs->index[cnt - 1] = initDataset[i].first;
          break;
        }
      }
    }
    cnt++;
  }
}

inline int BSModel::Predict(double key) const {
  int start_idx = 0;
  int end_idx = (flagNumber & 0x00FFFFFF) - 1;
  int mid;
  while (start_idx < end_idx) {
    mid = (start_idx + end_idx) / 2;
    if (index[mid] < key)
      start_idx = mid + 1;
    else
      end_idx = mid;
  }
  return start_idx;
}

#endif  // SRC_CARMI_NODES_INNERNODE_BS_MODEL_H_
