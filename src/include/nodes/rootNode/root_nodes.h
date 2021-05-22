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
#ifndef SRC_INCLUDE_NODES_ROOTNODE_ROOT_NODES_H_
#define SRC_INCLUDE_NODES_ROOTNODE_ROOT_NODES_H_

#include <fstream>
#include <vector>

#include "../../construct/structures.h"
#include "../../params.h"
#include "trainModel/linear_regression.h"

template <typename DataVectorType, typename DataType>
class LRType {
 public:
  LRType() = default;
  explicit LRType(int c) { flagNumber = (LR_ROOT_NODE << 24) + c; }
  LinearRegression<DataVectorType, DataType> model;  // 20 Byte
  int flagNumber;
  int childLeft;
};
#endif  // SRC_INCLUDE_NODES_ROOTNODE_ROOT_NODES_H_
