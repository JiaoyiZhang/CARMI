/**
 * @file dp_leaf.h
 * @author Jiaoyi
 * @brief use dynamic programming algorithm to construct leaf nodes
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_DP_LEAF_H_
#define SRC_INCLUDE_CONSTRUCT_DP_LEAF_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../carmi.h"
#include "../params.h"
#include "./structures.h"

template <typename KeyType, typename ValueType>
NodeCost CARMI<KeyType, ValueType>::DPLeaf(const DataRange &dataRange) {
  NodeCost nodeCost;
  NodeCost optimalCost = {DBL_MAX, DBL_MAX, DBL_MAX};
  BaseNode<KeyType, ValueType> optimal_node_struct;

  if (isPrimary) {
    nodeCost.time = 0.0;
    nodeCost.space = 0.0;

    ExternalArray<KeyType> tmp;
    tmp.Train(initDataset, dataRange.initRange.left, dataRange.initRange.size);
    auto error = tmp.error;
    int findEnd = dataRange.findRange.left + dataRange.findRange.size;
    for (int i = dataRange.findRange.left; i < findEnd; i++) {
      auto predict = tmp.Predict(findQuery[i].first) + dataRange.findRange.left;
      auto d = abs(i - predict);
      nodeCost.time +=
          (carmi_params::kLeafBaseTime * findQuery[i].second) / querySize;
      if (d <= error)
        nodeCost.time += (log2(error + 1) * findQuery[i].second *
                          carmi_params::kCostBSTime) /
                         querySize;
      else
        nodeCost.time += (log2(dataRange.initRange.size) * findQuery[i].second *
                          carmi_params::kCostBSTime) /
                         querySize;
    }

    nodeCost.cost =
        nodeCost.time + nodeCost.space * lambda;  // ns + MB * lambda
    optimalCost = {nodeCost.time, nodeCost.space, nodeCost.cost};
    optimal_node_struct.externalArray = tmp;

    auto it = COST.find(dataRange.initRange);
    if (it != COST.end()) {
      if (it->second.cost < optimalCost.cost) {
        return it->second;
      } else {
        COST.erase(dataRange.initRange);
        structMap.erase(dataRange.initRange);
      }
    }
    COST.insert({dataRange.initRange, optimalCost});
    structMap.insert({dataRange.initRange, optimal_node_struct});
    return nodeCost;
  }

  // choose a cf array node as the leaf node
  int totalDataNum = dataRange.initRange.size + dataRange.insertRange.size;
  int blockNum =
      CFArrayType<KeyType, ValueType>::CalNeededBlockNum(totalDataNum);
  int avgSlotNum = std::max(1.0, ceil(totalDataNum * 1.0 / blockNum));
  avgSlotNum =
      std::min(avgSlotNum, CFArrayType<KeyType, ValueType>::kMaxBlockCapacity);

  double time_cost = carmi_params::kLeafBaseTime;
  double space_cost = blockNum * carmi_params::kMaxLeafNodeSize;

  int end = dataRange.findRange.left + dataRange.findRange.size;
  for (int i = dataRange.findRange.left; i < end; i++) {
    time_cost += findQuery[i].second / querySize *
                 (log2(avgSlotNum) * carmi_params::kCostBSTime);
  }

  end = dataRange.insertRange.left + dataRange.insertRange.size;
  for (int i = dataRange.insertRange.left; i < end; i++) {
    time_cost += insertQuery[i].second / querySize *
                 ((log2(avgSlotNum) * carmi_params::kCostBSTime) +
                  (1 + avgSlotNum) / 2.0 * carmi_params::kCostMoveTime);
  }

  double cost = time_cost + space_cost * lambda;  // ns + MB * lambda
  if (cost <= optimalCost.cost) {
    optimalCost = {time_cost, space_cost, cost};
    optimal_node_struct.cfArray = CFArrayType<KeyType, ValueType>();
  }

  auto it = COST.find(dataRange.initRange);
  if (it != COST.end()) {
    if (it->second.cost < optimalCost.cost) {
      return it->second;
    } else {
      COST.erase(dataRange.initRange);
      structMap.erase(dataRange.initRange);
    }
  }
  COST.insert({dataRange.initRange, optimalCost});
  structMap.insert({dataRange.initRange, optimal_node_struct});
  return optimalCost;
}

#endif  // SRC_INCLUDE_CONSTRUCT_DP_LEAF_H_
