/**
 * @file array.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_NODES_LEAFNODE_LEAF_NODES_H_
#define SRC_INCLUDE_NODES_LEAFNODE_LEAF_NODES_H_

#include <float.h>

#include <iostream>
#include <utility>
#include <vector>

#include "../../construct/structures.h"
#include "../../params.h"

template <typename KeyType>
class ArrayType {
 public:
  ArrayType() {
    flagNumber = (ARRAY_LEAF_NODE << 24) + 0;
    m_left = -1;
    previousLeaf = -1;
    nextLeaf = -1;
    for (int i = 0; i < 48 / sizeof(KeyType); i++) {
      slotkeys[i] = DBL_MIN;
    }
  }

  /**
   * @brief predict the position of the given key
   *
   * @param key
   * @return int the predicted index in the leaf node
   */
  int Predict(KeyType key) const;

  int flagNumber;  // 4 Byte (flag + 0), size: the number of union
  int previousLeaf;
  int nextLeaf;
  int m_left;  // the left boundary of the leaf node in the global array

  KeyType slotkeys[48 / sizeof(KeyType)];
};

class ExternalArray {
 public:
  ExternalArray() = default;
  explicit ExternalArray(int cap) {
    flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + 0;
    error = 0;
    m_left = -1;
    theta1 = 0.0001;
    theta2 = 0.666;
  }
  inline int Predict(double key) const {
    // return the predicted idx in the leaf node
    int size = (flagNumber & 0x00FFFFFF);
    int p = (theta1 * key + theta2) * size;
    if (p < 0)
      p = 0;
    else if (p >= size && size != 0)
      p = size - 1;
    return p;
  }

  int flagNumber;  // 4 Byte (flag + 0)

  int m_left;  // the left boundary of the leaf node in the global array
  int error;   // the boundary of binary search

  float theta1;
  float theta2;

  float tmpMember;
  std::pair<float, float> tmppp[5];
};
#endif  // SRC_INCLUDE_NODES_LEAFNODE_LEAF_NODES_H_
