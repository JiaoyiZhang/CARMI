/**
 * @file his_model.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_HIS_MODEL_H_
#define SRC_CARMI_NODES_INNERNODE_HIS_MODEL_H_

#include <algorithm>
#include <utility>
#include <vector>

#include "../../carmi.h"

inline void CARMI::initHis(const DataVectorType &dataset, HisModel *his) {
  int childNumber = his->flagNumber & 0x00FFFFFF;
  his->childLeft = allocateChildMemory(childNumber);
  if (dataset.size() == 0) return;

  Train(0, dataset.size(), dataset, his);

  std::vector<DataVectorType> perSubDataset;
  DataVectorType tmp;
  for (int i = 0; i < childNumber; i++) perSubDataset.push_back(tmp);
  for (int i = 0; i < dataset.size(); i++) {
    int p = his->Predict(dataset[i].first);
    perSubDataset[p].push_back(dataset[i]);
  }

  switch (kLeafNodeID) {
    case ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        ArrayType tmp(kThreshold);
        initArray(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i],
                  &tmp);
        entireChild[his->childLeft + i].array = tmp;
      }
      break;
    case GAPPED_ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        GappedArrayType tmp(kThreshold);
        initGA(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i], &tmp);
        entireChild[his->childLeft + i].ga = tmp;
      }
      break;
  }
}

inline void CARMI::Train(const int left, const int size,
                         const DataVectorType &dataset, HisModel *his) {
  if (size == 0) return;
  int childNumber = his->flagNumber & 0x00FFFFFF;
  double maxValue;
  int end = left + size;
  for (int i = left; i < end; i++) {
    his->minValue = dataset[i].first;
    break;
  }
  for (int i = end - 1; i >= left; i--) {
    maxValue = dataset[i].first;
    break;
  }
  his->divisor = static_cast<float>(maxValue - his->minValue) / childNumber;
  std::vector<float> table(childNumber, 0);
  int cnt = 0;
  while (cnt <= 1) {
    if (cnt == 1) his->divisor /= 10;
    cnt = 0;
    table = std::vector<float>(childNumber, 0);
    for (int i = left; i < end; i++) {
      int idx =
          static_cast<float>(dataset[i].first - his->minValue) / his->divisor;
      idx = std::min(idx, static_cast<int>(table.size()) - 1);
      table[idx]++;
    }
    if (table[0] > 0) cnt++;
    table[0] = table[0] / size * childNumber;
    for (int i = 1; i < table.size(); i++) {
      if (table[i] > 0) cnt++;
      table[i] = table[i] / size * childNumber + table[i - 1];
    }
  }
  table[0] = round(table[0]);
  for (int i = 1; i < childNumber; i++) {
    table[i] = round(table[i]);
    if (table[i] - table[i - 1] > 1) table[i] = table[i - 1] + 1;
  }

  cnt = 0;
  for (int i = 0; i < childNumber; i += 16) {
    unsigned short start_idx = static_cast<int>(table[i]);
    unsigned short tmp = 0;
    for (int j = i; j < i + 16; j++) {
      if (j >= childNumber) {
        while (j < i + 16) {
          tmp = tmp << 1;
          j++;
        }
        his->Base[cnt] = table[i];
        his->Offset[cnt] = tmp;
        return;
      }
      unsigned short diff = static_cast<int>(table[j]) - start_idx;
      tmp = (tmp << 1) + diff;
#ifdef DEBUG
      if (diff > 1) std::cout << "diff wrong, diff:" << diff << std::endl;
#endif  // DEBUG
      if (diff > 0) start_idx += diff;
    }
    his->Base[cnt] = table[i];
    his->Offset[cnt++] = tmp;
  }
}

inline int HisModel::Predict(double key) const {
  // return the idx in children
  int childNumber = flagNumber & 0x00FFFFFF;
  int idx = (key - minValue) / divisor;
  if (idx < 0)
    idx = 0;
  else if (idx >= childNumber)
    idx = childNumber - 1;

  int base;
  int tmpIdx = idx / 16;
  base = Base[tmpIdx];
  int j = idx % 16;
  int tmp = Offset[tmpIdx] >> (15 - j);
  for (; j >= 0; j--) {  //-》1 2 4 8
    base += tmp & 1;
    tmp = tmp >> 1;
  }
  return base;
}

#endif  // SRC_CARMI_NODES_INNERNODE_HIS_MODEL_H_
