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
#ifndef SRC_CARMI_NODES_LEAFNODE_LEAF_NODES_H_
#define SRC_CARMI_NODES_LEAFNODE_LEAF_NODES_H_

#include <iostream>
#include <utility>
#include <vector>

#include "../../../params.h"
#include "../../construct/structures.h"

class ArrayType {
 public:
  ArrayType() = default;
  explicit ArrayType(int cap) {
    flagNumber = (ARRAY_LEAF_NODE << 24) + 0;
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

class GappedArrayType {
 public:
  GappedArrayType() = default;
  explicit GappedArrayType(int cap) {
    flagNumber = (GAPPED_ARRAY_LEAF_NODE << 24) + 0;
    error = 0;
    density = kDensity;
    capacity = cap;
    maxIndex = -2;
    m_left = -1;
    previousLeaf = -1;
    nextLeaf = -1;
  }
  int Predict(double key) const;

  int flagNumber;  // 4 Byte (flag + 0)
  int previousLeaf;
  int nextLeaf;

  int m_left;    // the left boundary of the leaf node in the global array
  int capacity;  // the current maximum capacity of the leaf node

  int maxIndex;   // tht index of the last one
  int error;      // the boundary of binary search
  float density;  // the maximum density of the leaf node data

  float theta1;
  float theta2;
  std::pair<float, float> tmppp[3];
};

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
#endif  // SRC_CARMI_NODES_LEAFNODE_LEAF_NODES_H_
