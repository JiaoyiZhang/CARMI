/**
 * @file baseNode.h
 * @author Jiaoyi
 * @brief the main structures of nodes
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef BASENODE_H_
#define BASENODE_H_
#include <iostream>
#include <utility>

#include "nodes/innerNode/bs_model.h"
#include "nodes/innerNode/his_model.h"
#include "nodes/innerNode/lr_model.h"
#include "nodes/innerNode/plr_model.h"
#include "nodes/leafNode/cfarray_type.h"
#include "nodes/leafNode/external_array_type.h"
#include "nodes/rootNode/root_nodes.h"

/**
 * @brief the root type of CARMI
 *
 * @tparam DataVectorType the type of data vector
 * @tparam KeyType  the type of the given key value
 */
template <typename DataVectorType, typename KeyType>
class CARMIRoot : public PLRType<DataVectorType, KeyType> {
 public:
  CARMIRoot() = default;
  /**
   * @brief Construct a new CARMIRoot object
   *
   * @param t set the type of the root node
   */
  explicit CARMIRoot(int t) { type = t; }
  CARMIRoot& operator=(const PLRType<DataVectorType, KeyType>& currnode) {
    this->PLRType<DataVectorType, KeyType>::model = currnode.model;
    this->PLRType<DataVectorType, KeyType>::fetch_model = currnode.fetch_model;
    this->flagNumber = currnode.flagNumber;
    this->childLeft = currnode.childLeft;
    this->type = currnode.flagNumber;
    return *this;
  }
  int type;  ///< the type of the root node
};

/**
 * @brief the 64 bytes structure for all types of nodes
 *
 * @tparam KeyType the type of the given key value
 */
template <typename KeyType, typename ValueType>
union BaseNode {
  LRModel<KeyType, ValueType> lr;    ///< linear regression inner node
  PLRModel<KeyType, ValueType> plr;  ///< piecewise linear regression inner node
  HisModel<KeyType, ValueType> his;  ///< histogram inner node
  BSModel<KeyType, ValueType> bs;    ///< binary search inner node

  CFArrayType<KeyType, ValueType> cfArray;  ///< cache-friendly array leaf node
  ExternalArray<KeyType> externalArray;     ///< external array leaf node

  BaseNode() {}
  ~BaseNode() {}
  BaseNode& operator=(const BaseNode& currnode) {
    if (this != &currnode) {
      this->lr = currnode.lr;
    }
    return *this;
  }
};

#endif  // BASENODE_H_
