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
#ifndef SRC_INCLUDE_NODES_INNERNODE_HIS_MODEL_H_
#define SRC_INCLUDE_NODES_INNERNODE_HIS_MODEL_H_

#include <algorithm>
#include <utility>
#include <vector>

#include "../../carmi.h"

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Train(const int left, const int size,
                                             const DataVectorType &dataset,
                                             HisModel *his) {
  if (size == 0) return;

  // calculate divisor
  int end = left + size;
  int childNumber = his->flagNumber & 0x00FFFFFF;
  double maxValue = dataset[end - 1].first;
  his->minValue = dataset[left].first;
  his->divisor = 1.0 / ((maxValue - his->minValue) / childNumber);

  // count the number of data points in each child
  std::vector<int> table(childNumber, 0);
  for (int i = left; i < end; i++) {
    int idx = (dataset[i].first - his->minValue) * his->divisor;
    idx = std::min(std::max(0, idx), childNumber - 1);
    table[idx]++;
  }

  // normalize table
  std::vector<double> index(childNumber, 0);
  index[0] = static_cast<double>(table[0]) / size * (childNumber - 1);
  for (int i = 1; i < childNumber; i++) {
    index[i] =
        static_cast<double>(table[i]) / size * (childNumber - 1) + index[i - 1];
  }

  table[0] = round(index[0]);
  for (int i = 1; i < childNumber; i++) {
    table[i] = round(index[i]);
    if (table[i] - table[i - 1] > 1) {
      table[i] = table[i - 1] + 1;
    }
  }

  // calculate the value of base and offset
  int cnt = 0;
  for (int i = 0; i < childNumber; i += 16) {
    unsigned short start_idx = table[i];
    unsigned short tmp = 0;
    for (int j = i; j < i + 16; j++) {
      if (j >= childNumber) {
        while (j < i + 16) {
          tmp = tmp << 1;
          j++;
        }
        his->base[cnt] = table[i];
        his->offset[cnt] = tmp;
        return;
      }
      unsigned short diff = table[j] - start_idx;
      tmp = (tmp << 1) + diff;

      if (diff > 0) {
        start_idx += diff;
      }
    }
    his->base[cnt] = table[i];
    his->offset[cnt++] = tmp;
  }
}

/**
 * @brief predict the index of the next branch
 *
 * @param key
 * @return int index (from 0 to childNumber-1 )
 */
inline int HisModel::Predict(double key) const {
  // return the idx in children
  int childNumber = flagNumber & 0x00FFFFFF;
  int idx = (key - minValue) * divisor;
  if (idx < 0)
    idx = 0;
  else if (idx >= childNumber)
    idx = childNumber - 1;

  int index = base[(idx >> 4)];
  int tmp = offset[(idx >> 4)] >> (15 - idx & 0x0000000F);
  tmp = (tmp & 0x55555555) + ((tmp >> 1) & 0x55555555);
  tmp = (tmp & 0x33333333) + ((tmp >> 2) & 0x33333333);
  tmp = (tmp & 0x0f0f0f0f) + ((tmp >> 4) & 0x0f0f0f0f);
  tmp = (tmp & 0x00ff00ff) + ((tmp >> 8) & 0x00ff00ff);
  index += tmp;

  return index;
}

#endif  // SRC_INCLUDE_NODES_INNERNODE_HIS_MODEL_H_
