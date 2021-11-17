/**
 * @file base_node.h
 * @author Jiaoyi
 * @brief the main structures of nodes
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef BASE_NODE_H_
#define BASE_NODE_H_
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <utility>
#include <vector>

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
 * first use the root node's model to compute the next node's index.  In the
 * CARMI framework, the object of this class serves as one of its private
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
   * @brief Construct a new CARMIRoot object with the default constructor
   */
  CARMIRoot() = default;

  /**
   * @brief Copy from a PLRType object to an object of the current object.
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
 * index of the child nodes in the node array. For leaf nodes, they store the
 * starting index of data blocks in the data array instead. The remaining 56
 * bytes store additional information depending on the tree node type.
 *
 * @tparam KeyType the type of the given key value
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType,
          typename Compare = std::less<KeyType>,
          typename Alloc = std::allocator<LeafSlots<KeyType, ValueType>>>
union BaseNode {
  /**
   * @brief the linear regression inner node
   */
  LRModel<KeyType, ValueType> lr;

  /**
   * @brief the piecewise linear regression inner node
   */
  PLRModel<KeyType, ValueType> plr;

  /**
   * @brief the histogram inner node
   */
  HisModel<KeyType, ValueType> his;

  /**
   * @brief the binary search inner node
   */
  BSModel<KeyType, ValueType> bs;

  /**
   * @brief the cache-friendly array leaf node
   */
  CFArrayType<KeyType, ValueType, Compare, Alloc> cfArray;

  /**
   * @brief the external array leaf node
   */
  ExternalArray<KeyType, ValueType, Compare> externalArray;

  BaseNode() {}
  ~BaseNode() {}

  BaseNode& operator=(const BaseNode& currnode) {
    if (this != &currnode) {
      this->lr = currnode.lr;
    }
    return *this;
  }
};

#endif  // BASE_NODE_H_
