/**
 * @file dp.h
 * @author Jiaoyi
 * @brief the main function of dynamic programming algorithm
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CONSTRUCT_DP_H_
#define CONSTRUCT_DP_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../params.h"
#include "./dp_inner.h"
#include "./dp_leaf.h"
#include "./greedy.h"
#include "./structures.h"

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
NodeCost CARMI<KeyType, ValueType, Compare, Alloc>::DP(const DataRange &range) {
  NodeCost nodeCost;
  // Case 1: the dataset is empty, construct an empty node and return directly
  if (range.initRange.size == 0) {
    nodeCost = emptyCost;
    // Construct an empty leaf node when the sub-dataset is empty and store
    // it in the structMap. The type of this leaf node depends on the isPrimary
    // parameter, if it is true, construct an external array leaf node,
    // otherwise, construct a cache-friendly array leaf node.
    BaseNode<KeyType, ValueType, Compare, Alloc> optimal_node_struct;
    if (isPrimary) {
      optimal_node_struct.externalArray =
          ExternalArray<KeyType, ValueType, Compare>();
    } else {
      optimal_node_struct.cfArray =
          CFArrayType<KeyType, ValueType, Compare, Alloc>();
    }
    structMap.insert({range.initRange, optimal_node_struct});
    return nodeCost;
  }

  // Case 2: this sub-dataset has been solved before, return the minimum cost
  // directly
  auto it = COST.find(range.initRange);
  if (it != COST.end()) {
    nodeCost = it->second;
    return nodeCost;
  }

  double minRatio = 0.95;
  // record the maximum capacity of the leaf node
  int maxStoredNum =
      CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxLeafCapacity;
  if (isPrimary) {
    maxStoredNum = carmi_params::kMaxLeafNodeSizeExternal;
  }
  if (range.initRange.size + range.insertRange.size <=
      minRatio * maxStoredNum) {
    // Case 3: if the size is smaller than the threshold, directly construct a
    // leaf node
    return DPLeaf(range);
  } else if (range.initRange.size + range.insertRange.size > maxStoredNum) {
    // Case 4: if the size is larger than the maximum capacity of a leaf node,
    // directly construct an inner node
    return DPInner(range);
  } else {
    // Case 5: construct a leaf node and an inner node respectively, and choose
    // the setting with a lower cost
    auto resInner = DPInner(range);
    auto resLeaf = DPLeaf(range);
    if (resInner.cost > resLeaf.cost)
      return resLeaf;
    else
      return resInner;
  }
}

#endif  // CONSTRUCT_DP_H_
