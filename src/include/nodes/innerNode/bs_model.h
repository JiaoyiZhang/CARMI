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
#ifndef SRC_INCLUDE_NODES_INNERNODE_BS_MODEL_H_
#define SRC_INCLUDE_NODES_INNERNODE_BS_MODEL_H_

#include <algorithm>
#include <utility>
#include <vector>

#include "../../carmi.h"

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Train(const int left, const int size,
                                             const DataVectorType &dataset,
                                             BSModel *bs) {
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

/**
 * @brief predict the index of the next branch
 *
 * @param key
 * @return int index (from 0 to childNumber-1 )
 */
inline int BSModel::Predict(double key) const {
  int start_idx = 0;
  int end_idx = (flagNumber & 0x00FFFFFF) - 1;
  int mid;
  while (start_idx < end_idx) {
    mid = (start_idx + end_idx) >> 1;
    if (index[mid] < key)
      start_idx = mid + 1;
    else
      end_idx = mid;
  }
  return start_idx;
}

#endif  // SRC_INCLUDE_NODES_INNERNODE_BS_MODEL_H_
