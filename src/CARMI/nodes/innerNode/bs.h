/**
 * @file bs.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_BS_H_
#define SRC_CARMI_NODES_INNERNODE_BS_H_
#include <algorithm>
#include <utility>
#include <vector>

#include "../../construct/structures.h"
class BSModel {
 public:
  BSModel() = default;
  void SetChildNumber(int c) {
    flagNumber = (BS_INNER_NODE << 24) + std::min(c, 15);
    for (int i = 0; i < 14; i++) index[i] = 0;
  }
  int Predict(double key) const;

  int flagNumber;   // 4 Byte (flag + childNumber)
  int childLeft;    // 4 Byte
  float index[14];  // 56 Byte (childNumber = 15)
};

#endif  // SRC_CARMI_NODES_INNERNODE_BS_H_
