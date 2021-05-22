/**
 * @file dp_leaf.h
 * @author Jiaoyi
 * @brief use dynamic programming algorithm to construct leaf nodes
 * @version 0.1
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

#include "../func/inlineFunction.h"
#include "../params.h"
#include "./structures.h"

template <typename KeyType, typename ValueType>
template <typename TYPE>
double CARMI<KeyType, ValueType>::CalLeafFindTime(
    int actualSize, double density, const TYPE &node,
    const IndexPair &range) const {
  double time_cost = 0;
  for (int i = range.left; i < range.left + range.size; i++) {
    auto predict = node.Predict(findQuery[i].first) + range.left;
    auto d = abs(i - predict);
    time_cost +=
        (carmi_params::kLeafBaseTime * findQuery[i].second) / querySize;
    if (d <= node.error)
      time_cost += (log2(node.error + 1) * findQuery[i].second *
                    carmi_params::kCostBSTime) *
                   (2 - density) / querySize;
    else
      time_cost += (log2(actualSize) * findQuery[i].second *
                    carmi_params::kCostBSTime * (2 - density)) /
                   querySize;
  }
  return time_cost;
}

template <typename KeyType, typename ValueType>
template <typename TYPE>
double CARMI<KeyType, ValueType>::CalLeafInsertTime(
    int actualSize, double density, const TYPE &node, const IndexPair &range,
    const IndexPair &findRange) const {
  double time_cost = 0;
  for (int i = range.left; i < range.left + range.size; i++) {
    int predict = node.Predict(insertQuery[i].first) + findRange.left;
    int d = abs(insertQueryIndex[i] - predict);
    time_cost +=
        (carmi_params::kLeafBaseTime * insertQuery[i].second) / querySize;
    // add the cost of binary search between error or the entire node
    if (d <= node.error)
      time_cost += (log2(node.error + 1) * insertQuery[i].second *
                    carmi_params::kCostBSTime) *
                   (2 - density) / querySize;
    else
      time_cost += (log2(actualSize) * insertQuery[i].second *
                    carmi_params::kCostBSTime * (2 - density)) /
                   querySize;
    // add the cost of moving data points
    if (density == 1)
      time_cost += carmi_params::kCostMoveTime * findRange.size / 2 *
                   insertQuery[i].second / querySize;
    else
      time_cost += carmi_params::kCostMoveTime * density / (1 - density) *
                   insertQuery[i].second / querySize;
  }
  return time_cost;
}

template <typename KeyType, typename ValueType>
NodeCost CARMI<KeyType, ValueType>::DPLeaf(const DataRange &dataRange) {
  NodeCost nodeCost;
  NodeCost optimalCost = {DBL_MAX, DBL_MAX, DBL_MAX};
  BaseNode optimal_node_struct;

  // TODO(jiaoyi): modify the process due to the change of leaf nodes
  if (isPrimary) {
    nodeCost.time = 0.0;
    nodeCost.space = 0.0;

    ExternalArray tmp(kThreshold);
    Train(dataRange.initRange.left, dataRange.initRange.size, initDataset,
          &tmp);
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

  int actualSize = dataRange.initRange.size;
  if (actualSize > carmi_params::kLeafMaxCapacity)
    actualSize = carmi_params::kLeafMaxCapacity;
  else
    actualSize = GetActualSize(actualSize);
  // choose an array node as the leaf node
  double time_cost = 0.0;
  double space_cost = kDataPointSize * actualSize;

  ArrayType tmp(actualSize);
  Train(dataRange.initRange.left, dataRange.initRange.size, initDataset, &tmp);
  time_cost +=
      CalLeafFindTime<ArrayType>(actualSize, 1, tmp, dataRange.findRange);
  time_cost += CalLeafInsertTime<ArrayType>(
      actualSize, 1, tmp, dataRange.insertRange, dataRange.findRange);

  double cost = time_cost + space_cost * lambda;  // ns + MB * lambda
  if (cost <= optimalCost.cost) {
    optimalCost = {time_cost, space_cost, cost};
    optimal_node_struct.array = tmp;
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
