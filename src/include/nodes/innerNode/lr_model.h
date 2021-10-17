/**
 * @file lr_model.h
 * @author Jiaoyi
 * @brief linear regression inner node
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_NODES_INNERNODE_LR_MODEL_H_
#define SRC_INCLUDE_NODES_INNERNODE_LR_MODEL_H_

#include <float.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "../../construct/structures.h"

/**
 * @brief linear regression inner node
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class LRModel {
 public:
  typedef std::pair<KeyType, ValueType> DataType;
  typedef std::vector<DataType> DataVectorType;
  LRModel() = default;
  /**
   * @brief Set the Child Number object
   *
   * @param c the number of child nodes
   */
  void SetChildNumber(int c) {
    childLeft = 0;
    flagNumber = (LR_INNER_NODE << 24) + c;
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

  int childLeft;          ///< 4 Byte, the start index of child nodes
  float slope;            ///< 4 Byte, the slpoe
  float intercept;        ///< 4 Byte, the intercept
  float Placeholder[12];  ///< 48 Byte, placeholder
};

template <typename KeyType, typename ValueType>
inline void LRModel<KeyType, ValueType>::Train(int left, int size,
                                               const DataVectorType &dataset) {
  if (size == 0) return;

  // extract data points and their index
  int childNumber = flagNumber & 0x00FFFFFF;
  DataVectorType currdata(size, {DBL_MAX, DBL_MAX});
  for (int i = 0, j = left; i < size; i++, j++) {
    currdata[i].first = dataset[j].first;
    currdata[i].second = static_cast<double>(i) / size * childNumber;
  }

  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = 0; i < size; i++) {
    t1 += currdata[i].first * currdata[i].first;
    t2 += currdata[i].first;
    t3 += currdata[i].first * currdata[i].second;
    t4 += currdata[i].second;
  }
  if (t1 * size - t2 * t2) {
    slope = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    intercept = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
  } else {
    slope = 0;
    intercept = 0;
  }
}

template <typename KeyType, typename ValueType>
inline int LRModel<KeyType, ValueType>::Predict(KeyType key) const {
  // return the predicted idx in the children
  int p = slope * key + intercept;
  int bound = flagNumber & 0x00FFFFFF;
  if (p < 0)
    p = 0;
  else if (p >= bound)
    p = bound - 1;
  return p;
}
#endif  // SRC_INCLUDE_NODES_INNERNODE_LR_MODEL_H_
