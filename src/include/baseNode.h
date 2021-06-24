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
#ifndef SRC_INCLUDE_BASENODE_H_
#define SRC_INCLUDE_BASENODE_H_
#include <iostream>
#include <utility>

#include "nodes/innerNode/inner_nodes.h"
#include "nodes/leafNode/leaf_nodes.h"
#include "nodes/rootNode/root_nodes.h"

template <typename DataVectorType, typename KeyType>
class CARMIRoot : public LRType<DataVectorType, KeyType> {
 public:
  CARMIRoot() = default;
  explicit CARMIRoot(int t) { type = t; }
  int type;
  CARMIRoot& operator=(const LRType<DataVectorType, KeyType>& node) {
    this->LRType<DataVectorType, KeyType>::model = node.model;
    this->LRType<DataVectorType, KeyType>::fetch_model = node.fetch_model;
    this->flagNumber = node.flagNumber;
    this->childLeft = node.childLeft;
    this->type = node.flagNumber;
    this->childNumber = node.childNumber;
    return *this;
  }
};

template <typename KeyType>
union BaseNode {
  LRModel lr;
  PLRModel plr;
  HisModel his;
  BSModel bs;

  ArrayType<KeyType> array;
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

#endif  // SRC_INCLUDE_BASENODE_H_
