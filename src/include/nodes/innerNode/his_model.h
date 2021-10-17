/**
 * @file his_model.h
 * @author Jiaoyi
 * @brief histogram inner node
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_NODES_INNERNODE_HIS_MODEL_H_
#define SRC_INCLUDE_NODES_INNERNODE_HIS_MODEL_H_

#include <math.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "../../construct/structures.h"

/**
 * @brief histogram inner node
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class HisModel {
 public:
  typedef std::pair<KeyType, ValueType> DataType;
  typedef std::vector<DataType> DataVectorType;
  HisModel() = default;
  /**
   * @brief Set the Child Number object
   *
   * @param c the number of child nodes
   */
  void SetChildNumber(int c) {
    childLeft = 0;
    flagNumber = (HIS_INNER_NODE << 24) + std::min(c, 256);
    minValue = 0;
    divisor = 1;
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
  int Predict(KeyType key) const;

  int flagNumber;  ///< 4 Byte (flag + childNumber)

  int childLeft;           ///< 4 Byte, the start index of child nodes
  float divisor;           ///< 4 Byte, used to determine the bucket
  float minValue;          ///< 4 Byte, minimum value of sub-dataset
  unsigned char base[16];  ///< 16 Byte, the index of the first bit of the
                           ///< corresponding 16 bits
  unsigned short
      offset[16];  ///< 32 Byte, the index difference (0 or 1) between buckets
};

template <typename KeyType, typename ValueType>
inline void HisModel<KeyType, ValueType>::Train(int left, int size,
                                                const DataVectorType &dataset) {
  if (size == 0) return;

  // calculate divisor
  int end = left + size;
  int childNumber = flagNumber & 0x00FFFFFF;
  double maxValue = dataset[end - 1].first;
  minValue = dataset[left].first;
  divisor = 1.0 / ((maxValue - minValue) / childNumber);

  // count the number of data points in each child
  std::vector<int> table(childNumber, 0);
  for (int i = left; i < end; i++) {
    int idx = (dataset[i].first - minValue) * divisor;
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
        base[cnt] = table[i];
        offset[cnt] = tmp;
        return;
      }
      unsigned short diff = table[j] - start_idx;
      tmp = (tmp << 1) + diff;

      if (diff > 0) {
        start_idx += diff;
      }
    }
    base[cnt] = table[i];
    offset[cnt++] = tmp;
  }
}

template <typename KeyType, typename ValueType>
inline int HisModel<KeyType, ValueType>::Predict(KeyType key) const {
  // return the idx in children
  int childNumber = flagNumber & 0x00FFFFFF;
  int idx = (key - minValue) * divisor;
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

#endif  // SRC_INCLUDE_NODES_INNERNODE_HIS_MODEL_H_
