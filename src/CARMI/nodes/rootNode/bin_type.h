/**
 * @file bin_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_ROOTNODE_BIN_TYPE_H_
#define SRC_CARMI_NODES_ROOTNODE_BIN_TYPE_H_

#include <vector>

#include "../../../params.h"
#include "../../construct/structures.h"
#include "trainModel/binary_search_model.h"

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

#endif  // SRC_CARMI_NODES_ROOTNODE_BIN_TYPE_H_
