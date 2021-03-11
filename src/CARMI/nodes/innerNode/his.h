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

class HisModel {
 public:
  HisModel() = default;
  void SetChildNumber(int c) {
    flagNumber = (6 << 24) + std::min(c, 160);
    minValue = 0;
    divisor = 1;
  }
  void Train(const std::vector<std::pair<double, double>> &dataset);
  int Predict(double key) const;

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;              // 4 Byte
  double divisor;             // 4 Byte (TODO: float)
  double minValue;            // 4 Byte (TODO: float)
  unsigned short Offset[10];  // 20 Byte (TODO:Offset[11])
  unsigned short Base[10];    // 20 Byte
};

#endif  // SRC_CARMI_NODES_INNERNODE_HIS_H_
