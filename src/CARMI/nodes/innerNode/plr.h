/**
 * @file plr.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_PLR_H_
#define SRC_CARMI_NODES_INNERNODE_PLR_H_

#include <iostream>
#include <utility>
#include <vector>

class PLRModel {
 public:
  PLRModel() = default;
  void SetChildNumber(int c) { flagNumber = (5 << 24) + c; }
  void Train(const std::vector<std::pair<double, double>> &dataset);
  int Predict(double key) const;

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;                   // 4 Byte
  std::pair<float, int> point[6];  // 48 Byte
  float maxX;
  float tmp;
};

#endif  // SRC_CARMI_NODES_INNERNODE_PLR_H_
