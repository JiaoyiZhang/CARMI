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
#include "../../construct/structures.h"
#include "trainModel/binary_search_model.h"
#include "trainModel/histogram.h"
#include "trainModel/linear_regression.h"
#include "trainModel/piecewiseLR.h"

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

class PLRType {
 public:
  PLRType() = default;
  explicit PLRType(int c) {
    flagNumber = (PLR_ROOT_NODE << 24) + c;
    model = new PiecewiseLR();
  }
  ~PLRType() { delete model; }

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;       // 4 Byte
  PiecewiseLR* model;  // 24*8+4 Byte
};

class HisType {
 public:
  HisType() {}
  explicit HisType(int c) : model(new HistogramModel(c)) {
    flagNumber = (HIS_ROOT_NODE << 24) + c;
  }
  ~HisType() { delete model; }

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;          // 4 Byte
  HistogramModel* model;  // 16+16c
  void operator=(const HisType& his) {
    childLeft = his.childLeft;
    model = his.model;
  }
};

class BSType {
 public:
  BSType() : model() {}
  explicit BSType(int c) : model(new BinarySearchModel(c)) {
    flagNumber = (BS_ROOT_NODE << 24) + c;
  }
  ~BSType() { delete model; }

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;             // 4c Byte + 4
  BinarySearchModel* model;  // 8c + 4
};
#endif  // SRC_CARMI_NODES_ROOTNODE_ROOT_NODES_H_
