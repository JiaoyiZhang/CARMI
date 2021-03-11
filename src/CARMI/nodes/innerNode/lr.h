/**
 * @file lr.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_LR_H_
#define SRC_CARMI_NODES_INNERNODE_LR_H_

#include <iostream>
#include <utility>
#include <vector>

class LRModel {
 public:
  LRModel() = default;
  void SetChildNumber(int c) { flagNumber = (4 << 24) + c; }
  void Train(const std::vector<std::pair<double, double>> &dataset);
  int Predict(double key) const;

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;                     // 4 Byte
  float divisor;                     // 4 Byte
  float minValue;                    // 4 Byte
  std::pair<float, float> theta[6];  // 48 Byte
};

#endif  // SRC_CARMI_NODES_INNERNODE_LR_H_
