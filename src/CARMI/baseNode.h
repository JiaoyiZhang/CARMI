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

union CARMIRoot {
  LRType lrRoot;
  PLRType plrRoot;
  HisType hisRoot;
  BSType bsRoot;
  CARMIRoot() {}
  ~CARMIRoot() {}
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
