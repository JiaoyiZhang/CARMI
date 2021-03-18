/**
 * @file ycsb_leaf.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_LEAFNODE_EXTERNAL_ARRAY_H_
#define SRC_CARMI_NODES_LEAFNODE_EXTERNAL_ARRAY_H_

#include <iostream>
#include <utility>
#include <vector>

#include "../../construct/structures.h"

class ExternalArray {
 public:
  // ExternalArray() = default;
  ExternalArray() {
    flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + 0;
    error = 0;
    m_left = -1;
  }
  inline int Predict(double key) const {
    // return the predicted idx in the leaf node
    int size = (flagNumber & 0x00FFFFFF);
    int p = (theta1 * key + theta2) * size;
    if (p < 0)
      p = 0;
    else if (p >= size)
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

#endif  // SRC_CARMI_NODES_LEAFNODE_EXTERNAL_ARRAY_H_
