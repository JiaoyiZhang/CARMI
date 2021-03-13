/**
 * @file his.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_HIS_H_
#define SRC_CARMI_NODES_INNERNODE_HIS_H_

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#include "../../construct/structures.h"

class HisModel {
 public:
  HisModel() = default;
  void SetChildNumber(int c) {
    flagNumber = (HIS_INNER_NODE << 24) + std::min(c, 256);
    minValue = 0;
    divisor = 1;
  }
  int Predict(double key) const;

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;             // 4 Byte
  float divisor;            // 4 Byte (TODO: float)
  float minValue;           // 4 Byte (TODO: float)
  unsigned char base[16];  // 16 Byte (TODO:Offset[11])
  unsigned short offset[16];   // 32 Byte
};

#endif  // SRC_CARMI_NODES_INNERNODE_HIS_H_
