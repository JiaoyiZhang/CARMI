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
#include <utility>
#include <vector>

#include "../../construct/structures.h"

/**
 * @brief linear regression inner node
 *
 * This class is the lr inner node, which use a linear regression to train the
 * model and predict the index of the next node. The CPU time cost of this
 * node is less than the other nodes.
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class LRModel {
 public:
  // *** Constructed Types and Constructor

  // the pair of data points
  typedef std::pair<KeyType, ValueType> DataType;

  // the vector of data points, which is the type of dataset
  typedef std::vector<DataType> DataVectorType;

  /**
   * @brief Construct a new LRModel object and use c to set its child number
   *
   * @param c[in] the number of its child nodes
   */
  explicit LRModel(int c) {
    childLeft = 0;
    slope = 0;
    intercept = 0;
    flagNumber = (LR_INNER_NODE << 24) + c;
  }

 public:
  // *** Basic Functions of LR Inner Node Objects

  /**
   * @brief train the linear regression model
   *
   * The training data points are stored in dataset[left, left + size].
   *
   * @param left[in] the starting index of data points
   * @param size[in]  the size of data points
   * @param dataset[in] used to train the model
   */
  void Train(int left, int size, const DataVectorType &dataset);

  /**
   * @brief predict the next node which manages the data point corresponding to
   * the given key value
   *
   * @param key[in] the given key value
   * @return int: the predicted index of next node
   */
  int Predict(KeyType key) const;

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

  // The slope parameter of the linear regression model. (4 bytes)
  float slope;

  // The intercept parameter of the linear regression model. (4 bytes)
  float intercept;

  /**
   * @brief Placeholder, used to make sure that the size of this node is 64
   * bytes. (48 bytes)
   *
   */
  int Placeholder[12];
};

template <typename KeyType, typename ValueType>
inline void LRModel<KeyType, ValueType>::Train(int left, int size,
                                               const DataVectorType &dataset) {
  // Case 1: the dataset is empty, return directly
  if (size == 0) return;

  // Case 2: use the dataset to train the model
  // extract data points from dataset[left, left + size] and use their processed
  // relative index as y to train
  int childNumber = flagNumber & 0x00FFFFFF;
  DataVectorType currdata(size, {DBL_MAX, DBL_MAX});
  for (int i = 0, j = left; i < size; i++, j++) {
    currdata[i].first = dataset[j].first;
    currdata[i].second = static_cast<double>(i) / size * childNumber;
  }

  // train the lr model
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
  // use the lr model to predict the index of the next node
  int p = slope * key + intercept;
  // get its child number
  int bound = flagNumber & 0x00FFFFFF;
  // check whether p exceeds the boundaries
  if (p < 0)
    p = 0;
  else if (p >= bound)
    p = bound - 1;
  return p;
}
#endif  // NODES_INNERNODE_LR_MODEL_H_
