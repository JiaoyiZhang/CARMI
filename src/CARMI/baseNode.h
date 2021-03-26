/**
 * @file baseNode.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_BASENODE_H_
#define SRC_CARMI_BASENODE_H_
#include <iostream>

#include "nodes/innerNode/inner_nodes.h"
#include "nodes/leafNode/leaf_nodes.h"
#include "nodes/rootNode/root_nodes.h"

class CARMIRoot : public LRType, public PLRType, public HisType, public BSType {
 public:
  CARMIRoot() = default;
  explicit CARMIRoot(int t) { type = t; }
  int type;
  CARMIRoot& operator=(const LRType& node) {
    this->LRType::model = node.model;
    this->flagNumber = node.flagNumber;
    this->childLeft = node.childLeft;
    return *this;
  }
  CARMIRoot& operator=(const PLRType& node) {
    this->PLRType::model = node.model;
    this->flagNumber = node.flagNumber;
    this->childLeft = node.childLeft;
    return *this;
  }
  CARMIRoot& operator=(const HisType& node) {
    this->HisType::model = node.model;
    this->flagNumber = node.flagNumber;
    this->childLeft = node.childLeft;
    return *this;
  }
  CARMIRoot& operator=(const BSType& node) {
    this->BSType::model = node.model;
    this->flagNumber = node.flagNumber;
    this->childLeft = node.childLeft;
    return *this;
  }
};

union BaseNode {
  LRModel lr;
  PLRModel plr;
  HisModel his;
  BSModel bs;

  ArrayType array;
  GappedArrayType ga;
  ExternalArray externalArray;

  BaseNode() {}
  ~BaseNode() {}
  BaseNode& operator=(const BaseNode& node) {
    if (this != &node) {
      this->lr = node.lr;
    }
    return *this;
  }
};

#endif  // SRC_CARMI_BASENODE_H_
