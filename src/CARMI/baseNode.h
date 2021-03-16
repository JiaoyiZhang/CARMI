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

#include "nodes/innerNode/bs.h"
#include "nodes/innerNode/his.h"
#include "nodes/innerNode/lr.h"
#include "nodes/innerNode/plr.h"
#include "nodes/leafNode/array.h"
#include "nodes/leafNode/ga.h"
#include "nodes/leafNode/ycsb_leaf.h"
#include "nodes/rootNode/bin_type.h"
#include "nodes/rootNode/his_type.h"
#include "nodes/rootNode/lr_type.h"
#include "nodes/rootNode/plr_type.h"

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
  YCSBLeaf ycsbLeaf;

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
