/**
 * @file dp_leaf.h
 * @author Jiaoyi
 * @brief use dynamic programming algorithm to construct a leaf node
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CONSTRUCT_DP_LEAF_H_
#define CONSTRUCT_DP_LEAF_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../carmi.h"
#include "../params.h"
#include "./structures.h"

template <typename KeyType, typename ValueType>
NodeCost CARMI<KeyType, ValueType>::DPLeaf(const DataRange &dataRange) {
  NodeCost nodeCost{DBL_MAX, DBL_MAX, DBL_MAX};
  BaseNode<KeyType, ValueType> optimal_node_struct;

  if (isPrimary) {
    // construct an external array leaf node as the current node
    nodeCost.time = 0.0;
    nodeCost.space = 0.0;

    ExternalArray<KeyType> tmp;
    tmp.Train(initDataset, dataRange.initRange.left, dataRange.initRange.size);
    int findEnd = dataRange.findRange.left + dataRange.findRange.size;
    // calculate the time cost of this external array leaf node
    for (int i = dataRange.findRange.left; i < findEnd; i++) {
      int p = tmp.Predict(findQuery[i].first) + dataRange.findRange.left;
      int d = abs(i - p);
      nodeCost.time +=
          (carmi_params::kLeafBaseTime * findQuery[i].second) / querySize;
      // Case 1: if the data point is within the error range, perform binary
      // search over the range of [p - error / 2, p + error / 2]
      if (d <= tmp.error)
        nodeCost.time += log2(tmp.error + 1) * findQuery[i].second *
                         carmi_params::kCostBSTime / querySize;
      // Case 2: the data point is not in the error range, perform binary search
      // over the entire sub-dataset
      else
        nodeCost.time += log2(dataRange.initRange.size) * findQuery[i].second *
                         carmi_params::kCostBSTime / querySize;
    }
    optimal_node_struct.externalArray = tmp;

  } else {
    // choose a cf array node as the leaf node
    int totalDataNum = dataRange.initRange.size + dataRange.insertRange.size;
    // calculate the number of needed data blocks
    int blockNum =
        CFArrayType<KeyType, ValueType>::CalNeededBlockNum(totalDataNum);
    int avgSlotNum = std::max(1.0, ceil(totalDataNum * 1.0 / blockNum));
    avgSlotNum = std::min(avgSlotNum,
                          CFArrayType<KeyType, ValueType>::kMaxBlockCapacity);

    nodeCost.time = carmi_params::kLeafBaseTime;
    nodeCost.space = blockNum * carmi_params::kMaxLeafNodeSize;
    // calculate the time cost of find operations
    int end = dataRange.findRange.left + dataRange.findRange.size;
    for (int i = dataRange.findRange.left; i < end; i++) {
      nodeCost.time += findQuery[i].second * 1.0 / querySize *
                       (log2(avgSlotNum) * carmi_params::kCostBSTime);
    }
    // calculate the time cost of insert operations
    end = dataRange.insertRange.left + dataRange.insertRange.size;
    for (int i = dataRange.insertRange.left; i < end; i++) {
      nodeCost.time += 1.0 / querySize *
                       ((log2(avgSlotNum) * carmi_params::kCostBSTime) +
                        (1 + avgSlotNum) / 2.0 * carmi_params::kCostMoveTime);
    }
    optimal_node_struct.cfArray = CFArrayType<KeyType, ValueType>();
  }
  nodeCost.cost = nodeCost.time + nodeCost.space * lambda;

  // if dp algorithm also constructs an inner node on this sub-dataset, we need
  // to check which one is the better setting
  auto it = COST.find(dataRange.initRange);
  if (it != COST.end()) {
    if (it->second.cost < nodeCost.cost) {
      // Case 1: the inner node is the better one, return the cost of it
      // directly.
      return nodeCost;
    } else {
      // Case 2: the leaf node is the better one, erase the cost and the setting
      // of the inner node
      COST.erase(dataRange.initRange);
      structMap.erase(dataRange.initRange);
    }
  }
  // store the optimal cost and setting
  COST.insert({dataRange.initRange, nodeCost});
  structMap.insert({dataRange.initRange, optimal_node_struct});
  return nodeCost;
}

#endif  // CONSTRUCT_DP_LEAF_H_
