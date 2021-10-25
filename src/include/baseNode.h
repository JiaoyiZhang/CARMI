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
 * This class inherits PLRType as the root node. When accessing a data point, we
 * first use the model of the root node to compute the index of the next node.
 * In the CARMI framework, the object of this class serves as one of its private
 * members.
 *
 * @tparam DataVectorType the type of data vector
 * @tparam KeyType the type of the given key value
 */
template <typename DataVectorType, typename KeyType>
class CARMIRoot : public PLRType<DataVectorType, KeyType> {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief Construct a new CARMIRoot object
   */
  CARMIRoot() = default;

  /**
   * @brief Copy from a PLRType object to an object of the current class.
   *
   * @param[in] currnode the PLRType object
   * @return CARMIRoot& the object of the current class
   */
  CARMIRoot& operator=(const PLRType<DataVectorType, KeyType>& currnode) {
    this->PLRType<DataVectorType, KeyType>::model = currnode.model;
    this->PLRType<DataVectorType, KeyType>::fetch_model = currnode.fetch_model;
    this->flagNumber = currnode.flagNumber;
    return *this;
  }
};

/**
 * @brief the 64 bytes structure for all types of nodes to support the
 * cache-aware design
 *
 * The first byte is always the node type identifier, and the next three bytes
 * are used to store the number of child nodes (the number of data blocks for
 * leaf nodes). For inner nodes, the following 4 bytes represent the starting
 * index of the child nodes in node array. For leaf nodes, they store the
 * starting index of data blocks in data array instead. The remaining 56 bytes
 * store additional information depending on the tree node type.
 *
 * @tparam KeyType the type of the given key value
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
union BaseNode {
  /**
   * @brief linear regression inner node
   */
  LRModel<KeyType, ValueType> lr;

  /**
   * @brief piecewise linear regression inner node
   */
  PLRModel<KeyType, ValueType> plr;

  /**
   * @brief histogram inner node
   */
  HisModel<KeyType, ValueType> his;

  /**
   * @brief binary search inner node
   */
  BSModel<KeyType, ValueType> bs;

  /**
   * @brief cache-friendly array leaf node
   */
  CFArrayType<KeyType, ValueType> cfArray;

  /**
   * @brief external array leaf node
   */
  ExternalArray<KeyType> externalArray;

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
