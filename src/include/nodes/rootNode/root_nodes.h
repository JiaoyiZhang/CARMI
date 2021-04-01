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
#ifndef SRC_CARMI_NODES_ROOTNODE_ROOT_NODES_H_
#define SRC_CARMI_NODES_ROOTNODE_ROOT_NODES_H_

#include <fstream>
#include <vector>

#include "../../../params.h"
#include "trainModel/binary_search_model.h"
#include "trainModel/histogram.h"
#include "trainModel/linear_regression.h"
#include "trainModel/piecewiseLR.h"

class BaseRoot {
 public:
  int flagNumber;
  int childLeft;
};

class LRType : virtual public BaseRoot {
 public:
  LRType() = default;
  explicit LRType(int c) { flagNumber = (LR_ROOT_NODE << 24) + c; }
  LinearRegression model;  // 20 Byte
};

class PLRType : virtual public BaseRoot {
 public:
  PLRType() = default;
  explicit PLRType(int c) { flagNumber = (PLR_ROOT_NODE << 24) + c; }
  PiecewiseLR model;  // 24*8+4 Byte
};

class HisType : virtual public BaseRoot {
 public:
  HisType() = default;
  explicit HisType(int c) : model(HistogramModel(c)) {
    flagNumber = (HIS_ROOT_NODE << 24) + c;
  }
  HistogramModel model;  // 16+16c
};

class BSType : virtual public BaseRoot {
 public:
  BSType() = default;
  explicit BSType(int c) : model(BinarySearchModel(c)) {
    flagNumber = (BS_ROOT_NODE << 24) + c;
  }
  BinarySearchModel model;  // 8c + 4
};
#endif  // SRC_CARMI_NODES_ROOTNODE_ROOT_NODES_H_
