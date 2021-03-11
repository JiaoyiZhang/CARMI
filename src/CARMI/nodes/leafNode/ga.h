/**
 * @file ga.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_LEAFNODE_GA_H_
#define SRC_CARMI_NODES_LEAFNODE_GA_H_

#include <iostream>
#include <utility>
#include <vector>

#include "../../../params.h"

class GappedArrayType {
 public:
  GappedArrayType() = default;
  explicit GappedArrayType(int cap) {
    flagNumber = (9 << 24) + 0;
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

#endif  // SRC_CARMI_NODES_LEAFNODE_GA_H_
