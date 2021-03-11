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
#ifndef SRC_CARMI_NODES_LEAFNODE_ARRAY_H_
#define SRC_CARMI_NODES_LEAFNODE_ARRAY_H_

#include <iostream>
#include <utility>
#include <vector>

class ArrayType {
 public:
  ArrayType() = default;
  explicit ArrayType(int cap) {
    flagNumber = (8 << 24) + 0;
    error = 0;
    m_left = -1;
    m_capacity = cap;
    previousLeaf = -1;
    nextLeaf = -1;
  }
  inline int Predict(double key) const;

  int flagNumber;  // 4 Byte (flag + 0)
  int previousLeaf;
  int nextLeaf;

  int m_left;      // the left boundary of the leaf node in the global array
  int m_capacity;  // the maximum capacity of this leaf node
  int error;       // the boundary of binary search

  float theta1;
  float theta2;
  std::pair<float, float> tmppp[4];
};

#endif  // SRC_CARMI_NODES_LEAFNODE_ARRAY_H_
