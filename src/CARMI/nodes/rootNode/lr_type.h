/**
 * @file lr_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_ROOTNODE_LR_TYPE_H_
#define SRC_CARMI_NODES_ROOTNODE_LR_TYPE_H_

#include <fstream>
#include <vector>

#include "../../../params.h"
#include "../../construct/structures.h"
#include "trainModel/linear_regression.h"

class LRType {
 public:
  LRType() = default;
  explicit LRType(int c) {
    flagNumber = (LR_ROOT_NODE << 24) + c;
    model = new LinearRegression();
  }
  ~LRType() { delete model; }

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;            // 4 Byte
  LinearRegression* model;  // 20 Byte
};

#endif  // SRC_CARMI_NODES_ROOTNODE_LR_TYPE_H_
