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
#ifndef NODES_INNERNODE_LR_MODEL_H_
#define NODES_INNERNODE_LR_MODEL_H_

#include <float.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../../construct/structures.h"

/**
 * @brief linear regression inner node
 *
 * This class is the LR inner node, which uses linear regression to train the
 * model and predict the index of the next node. The CPU time cost of this node
 * is less than the other nodes.
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class LRModel {
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
   * @brief Construct a new LRModel object and use c to set its child number
   *
   * @param[in] c the number of its child nodes
   */
  explicit LRModel(int c) {
    childLeft = 0;
    slope = 0;
    intercept = 0;
    minValue = 0;
    flagNumber = (LR_INNER_NODE << 24) + std::max(std::min(c, 0x00FFFFFF), 2);
  }

 public:
  // *** Basic Functions of LR Inner Node Objects

  /**
   * @brief train the linear regression model
   *
   * The training data points are stored in dataset[left, left + size].
   *
   * @param[in] left the starting index of data points
   * @param[in] size  the size of data points
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
  // *** Static Constant Options and Values of LR Inner Node Objects

  /**
   * @brief The time cost of the lr inner node.
   */
  static constexpr double kTimeCost = carmi_params::kLRInnerTime;

  /**
   * @brief The bytes of placeholder.
   */
  static constexpr int kPlaceHolderLen = 48 - sizeof(KeyType);

 public:
  //*** Public Data Members of LR Inner Node Objects

  /**
   * @brief A combined integer, composed of the flag of lr inner node
   * (LR_INNER_NODE, 1 byte) and the number of its child nodes (3 bytes). (This
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
   * @brief The slope parameter of the linear regression model. (4 bytes)
   */
  float slope;

  /**
   * @brief The intercept parameter of the linear regression model. (4 bytes)
   */
  float intercept;

  /**
   * @brief The minimum value.
   */
  KeyType minValue;

  /**
   * @brief Placeholder, used to make sure that the size of this node is 64
   * bytes. (kPlaceHolderLen bytes)
   */
  char Placeholder[kPlaceHolderLen];
};

template <typename KeyType, typename ValueType>
inline void LRModel<KeyType, ValueType>::Train(int left, int size,
                                               const DataVectorType &dataset) {
  // Case 1: the dataset is empty, return directly
  if (size == 0) return;
  if (left < 0 || size < 0 || left + size > dataset.size()) {
    throw std::out_of_range(
        "LRModel::Train: the range of training dataset is invalid.");
  }

  // Case 2: use the dataset to train the model
  // extract data points from dataset[left, left + size] and use their processed
  // relative index as y to train
  int childNumber = flagNumber & 0x00FFFFFF;
  minValue = dataset[left].first;
  std::vector<std::pair<KeyType, double>> currdata(size);
  for (int i = 0, j = left; i < size; i++, j++) {
    currdata[i].first = dataset[j].first - minValue;
    currdata[i].second = i * 1.0 / size * childNumber;
  }

  // train the lr model
  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = 0; i < size; i++) {
    t1 += static_cast<double>(currdata[i].first) *
          static_cast<double>(currdata[i].first);
    t2 += static_cast<double>(currdata[i].first);
    t3 += static_cast<double>(currdata[i].first) * currdata[i].second;
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
  // use the lr model to predict the index of the next node
  int p = slope * static_cast<double>(key - minValue) + intercept;
  // get its child number
  int bound = flagNumber & 0x00FFFFFF;
  // check whether p exceeds the boundaries
  if (p < 0)
    p = 0;
  else if (p >= bound)
    p = bound - 1;
  return p + childLeft;
}
#endif  // NODES_INNERNODE_LR_MODEL_H_
