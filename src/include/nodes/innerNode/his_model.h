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
#ifndef NODES_INNERNODE_HIS_MODEL_H_
#define NODES_INNERNODE_HIS_MODEL_H_

#include <math.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "../../construct/structures.h"

/**
 * @brief histogram inner node
 *
 * This class is the his inner node, which use a histogram to train the model
 * and predict the index of the next node. In order to manage more child nodes
 * without exceeding the size limit of 64 bytes, we use the starting index of
 * each group and the relative difference in bits to represent the index of each
 * child node.
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class HisModel {
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
   * @brief Construct a new His Model object and use c to set its child number
   *
   * @param c[in] the number of its child nodes
   */
  explicit HisModel(int c) {
    childLeft = 0;
    flagNumber = (HIS_INNER_NODE << 24) + std::min(c, 256);
    minValue = 0;
    divisor = 1;
  }

 public:
  // *** Basic Functions of His Inner Node Objects

  /**
   * @brief train the histogram model
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
  // *** Static Constant Options and Values of His Inner Node Objects

  /**
   * @brief The time cost of the lr inner node.
   */
  static constexpr int kTimeCost = carmi_params::kHisInnerTime;

 public:
  //*** Public Data Members of His Inner Node Objects

  /**
   * @brief A combined integer, composed of the flag of lr inner node
   * (HIS_INNER_NODE, 1 byte) and the number of its child nodes (3 bytes). (This
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
   * @brief The divisor, used to determine the bucket.
   * The index of the item in the base is equal to static_cast<int>((key -
   * minValue) / divisor). (4 bytes)
   */
  float divisor;

  /**
   * @brief The minimum value of the subdataset, used to determine the bucket.
   * The index of the item in the base is equal to static_cast<int>((key -
   * minValue) / divisor). (4 bytes)
   */
  float minValue;

  /**
   * @brief Each byte represents the index in child nodes of the first bit of
   * the corresponding 16 bits in offset. When predicting the index of the next
   * node, we need to visit this member to get the basic index. (16 bytes)
   */
  unsigned char base[16];

  /**
   * @brief Each bit of offset represents the difference (0 or 1) between the
   * index of the current bucket and the previous bucket. After obtaining the
   * basic index, then count the number of bits in the offset table, and finally
   * add them together to get the index of the next node. (32 bytes)
   */
  unsigned short offset[16];
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
  int childNumber = flagNumber & 0x00FFFFFF;
  // calculate the index of the corresponding bucket
  int idx = (key - minValue) * divisor;
  if (idx < 0)
    idx = 0;
  else if (idx >= childNumber)
    idx = childNumber - 1;

  // get the basic index
  int index = base[(idx >> 4)];
  // count the number of bits in the offset table
  int tmp = offset[(idx >> 4)] >> (15 - (idx & 0x0000000F));
  tmp = (tmp & 0x55555555) + ((tmp >> 1) & 0x55555555);
  tmp = (tmp & 0x33333333) + ((tmp >> 2) & 0x33333333);
  tmp = (tmp & 0x0f0f0f0f) + ((tmp >> 4) & 0x0f0f0f0f);
  tmp = (tmp & 0x00ff00ff) + ((tmp >> 8) & 0x00ff00ff);

  // add them together to get the index of the next node
  index += tmp;
  return index;
}

#endif  // NODES_INNERNODE_HIS_MODEL_H_
