/**
 * @file bs_model.h
 * @author Jiaoyi
 * @brief binary search inner node
 * @version 3.0
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

#include "../../construct/structures.h"

/**
 * @brief binary search inner node
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class BSModel {
 public:
  typedef std::pair<KeyType, ValueType> DataType;
  typedef std::vector<DataType> DataVectorType;
  BSModel() = default;

  /**
   * @brief Set the Child Number object
   *
   * @param c the number of child nodes
   */
  void SetChildNumber(int c) {
    childLeft = 0;
    flagNumber = (BS_INNER_NODE << 24) + std::min(c, 15);
    for (int i = 0; i < 14; i++) index[i] = 0;
  }

  /**
   * @brief train the model with this dataset
   *
   * @param left the start index of data points
   * @param size  the size of data points
   * @param dataset used to train the model
   */
  void Train(int left, int size, const DataVectorType &dataset);

  /**
   * @brief predict the position of the given key value
   *
   * @param key the given key value
   * @return int: the predicted index of next node
   */
  int Predict(double key) const;

  int flagNumber;   ///< 4 Byte (flag + childNumber)
  int childLeft;    ///< 4 Byte, the start index of child nodes
  float index[14];  ///< 56 Byte, the minimum values of each segment
};

template <typename KeyType, typename ValueType>
inline void BSModel<KeyType, ValueType>::Train(int left, int size,
                                               const DataVectorType &dataset) {
  if (size == 0) return;
  int childNumber = flagNumber & 0x00FFFFFF;
  float value = static_cast<float>(size) / childNumber;
  int cnt = 1;
  int start = std::min(static_cast<float>(left), left + value * cnt - 1);
  int end = left + size;
  for (int i = start; i < end; i += value) {
    if (cnt >= childNumber) break;
    if (dataset[i].first != -DBL_MAX) {
      index[cnt - 1] = static_cast<int>(dataset[i].first);
      index[cnt - 1] += static_cast<float>(dataset[i].first - index[cnt - 1]);
    } else {
      for (int j = i + 1; j < end; j++) {
        if (dataset[j].first != -DBL_MAX) {
          index[cnt - 1] = dataset[i].first;
          break;
        }
      }
    }
    cnt++;
  }
}

template <typename KeyType, typename ValueType>
inline int BSModel<KeyType, ValueType>::Predict(double key) const {
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
