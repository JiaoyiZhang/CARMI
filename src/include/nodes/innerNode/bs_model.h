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
#ifndef NODES_INNERNODE_BS_MODEL_H_
#define NODES_INNERNODE_BS_MODEL_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../../construct/structures.h"

/**
 * @brief binary search inner node
 *
 * This class is the binary search inner node. Due to the size limit of 64
 * bytes, we can only store the kMaxKeyNum key values. Thus, this type is not
 * suitable for nodes with a large number of child nodes. However, the bs node
 * can divide the dataset evenly, thus dealing with the uneven dataset.
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class BSModel {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief the pair of data points
   */
  typedef std::pair<KeyType, ValueType> DataType;

  /**
   * @brief the vector of data points, which is the type of dataset
   */
  typedef std::vector<DataType> DataVectorType;

  /**
   * @brief Construct a new BS Model object and use c to set its child number
   *
   * This model is a binary search model, which performs a binary search between
   * the index vector to find the index of the given key value, and the size of
   * the index must be less than 14 due to the limit of 64 bytes.
   *
   * @param[in] c the number of its child nodes
   */
  explicit BSModel(int c) {
    childLeft = 0;
    flagNumber =
        (BS_INNER_NODE << 24) + std::max(2, std::min(c, kMaxKeyNum + 1));
    for (int i = 0; i < kMaxKeyNum; i++) {
      keys[i] = 0;
    }
  }

 public:
  // *** Basic Functions of BS Inner Node Objects

  /**
   * @brief train the binary search model
   *
   * The training data points are stored in dataset[left, left + size].
   *
   * @param[in] left the starting index of data points
   * @param[in] size the size of data points
   * @param[in] dataset used to train the model
   */
  void Train(int left, int size, const DataVectorType &dataset);

  /**
   * @brief predict the next node which manages the data point corresponding to
   * the given key value
   *
   * @param[in] key the given key value
   * @return int: the predicted index of next node
   */
  int Predict(KeyType key) const;

 public:
  // *** Static Constant Options and Values of BS Inner Node Objects

  /**
   * @brief The time cost of the bs inner node.
   */
  static constexpr double kTimeCost = carmi_params::kBSInnerTime;

  /**
   * @brief The maximum number of stored keys.
   */
  static constexpr int kMaxKeyNum = 56 / sizeof(KeyType);

 public:
  //*** Public Data Members of BS Inner Node Objects

  /**
   * @brief A combined integer, composed of the flag of bs inner node
   * (BS_INNER_NODE, 1 byte) and the number of its child nodes (3 bytes). (This
   * member is 4 bytes)
   */
  int flagNumber;

  /**
   * @brief The index of its first child node in the node array. All the child
   * nodes are stored in node[childLeft, childLeft + size]. Through this member
   * and the right three bytes of flagNumber, all the child nodes can be
   * accessed. (4 bytes)
   */
  int childLeft;

  /**
   * @brief store at most kMaxKeyNum key values
   * This bs model divides the key range into kMaxKeyNum + 1 intervals. To
   * determine which branch to go through, perform a binary search among the
   * kMaxKeyNum key values to locate the corresponding key value interval
   * covering the input key. (56 bytes)
   */
  KeyType keys[kMaxKeyNum];
};

template <typename KeyType, typename ValueType>
inline void BSModel<KeyType, ValueType>::Train(int left, int size,
                                               const DataVectorType &dataset) {
  if (size == 0) return;
  if (left < 0 || size < 0 || left + size > dataset.size()) {
    throw std::out_of_range(
        "BSModel::Train: the range of training dataset is invalid.");
  }

  int childNumber = flagNumber & 0x00FFFFFF;
  // calculate the value of the segment
  float value = static_cast<float>(size) / childNumber;
  int cnt = 1;
  int start = left + value;
  int end = left + size;
  // store the minimum value of each segment
  for (int i = start; i < end; i += value) {
    if (cnt >= childNumber) {
      break;
    }
    keys[cnt - 1] = dataset[i].first;
    cnt++;
  }
}

template <typename KeyType, typename ValueType>
inline int BSModel<KeyType, ValueType>::Predict(KeyType key) const {
  int start_idx = 0;
  // get the maximum index
  int end_idx = (flagNumber & 0x00FFFFFF) - 2;
  if (key > keys[end_idx]) {
    return childLeft + end_idx + 1;
  }
  int mid;
  // perform binary search between the index vector
  while (start_idx < end_idx) {
    mid = (start_idx + end_idx) >> 1;
    if (keys[mid] < key)
      start_idx = mid + 1;
    else
      end_idx = mid;
  }
  return start_idx + childLeft;
}

#endif  // NODES_INNERNODE_BS_MODEL_H_
