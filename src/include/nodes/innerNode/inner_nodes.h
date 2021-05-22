/**
 * @file inner_nodes.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-18
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SRC_INCLUDE_NODES_INNERNODE_INNER_NODES_H_
#define SRC_INCLUDE_NODES_INNERNODE_INNER_NODES_H_

#include <algorithm>
#include <utility>
#include <vector>

#include "../../construct/structures.h"
#include "../../params.h"

class LRModel {
 public:
  LRModel() = default;
  void SetChildNumber(int c) { flagNumber = (LR_INNER_NODE << 24) + c; }
  int Predict(double key) const;

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;                     // 4 Byte
  float divisor;                     // 4 Byte
  float minValue;                    // 4 Byte
  std::pair<float, float> theta[6];  // 48 Byte
};

class PLRModel {
 public:
  PLRModel() = default;
  void SetChildNumber(int c) { flagNumber = (PLR_INNER_NODE << 24) + c; }
  int Predict(double key) const;

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;  // 4 Byte
  int index[6];   // 24 Byte
  float keys[8];  // 32 Byte
};

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

  int childLeft;              // 4 Byte
  float divisor;              // 4 Byte
  float minValue;             // 4 Byte
  unsigned char base[16];     // 16 Byte
  unsigned short offset[16];  // 32 Byte
};

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
#endif  // SRC_INCLUDE_NODES_INNERNODE_INNER_NODES_H_
