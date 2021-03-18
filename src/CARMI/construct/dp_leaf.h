/**
 * @file dp_leaf.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_CONSTRUCT_DP_LEAF_H_
#define SRC_CARMI_CONSTRUCT_DP_LEAF_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../../params.h"
#include "../func/inlineFunction.h"
#include "./structures.h"

/**
 * @brief calculate the time cost of find queries
 *
 * @tparam TYPE the type of this leaf node
 * @param actualSize the capacity of this leaf node
 * @param density the density of of this leaf node (array: 1)
 * @param node this leaf node
 * @param range the range of find queries
 * @return double the time cost of this leaf node
 */
template <typename TYPE>
double CARMI::CalLeafFindTime(int actualSize, double density, const TYPE &node,
                              const IndexPair &range) const {
  double time_cost = 0;
  for (int i = range.left; i < range.left + range.size; i++) {
    auto predict = node.Predict(findQuery[i].first) + range.left;
    auto d = abs(i - predict);
    time_cost += (kCostBaseTime * findQuery[i].second) / querySize;
    if (d <= node.error)
      time_cost += (log2(node.error + 1) * findQuery[i].second * kCostBSTime) *
                   (2 - density) / querySize;
    else
      time_cost += (log2(actualSize) * findQuery[i].second * kCostBSTime *
                    (2 - density)) /
                   querySize;
  }
  return time_cost;
}

/**
 * @brief calculate the time cost of insert queries
 *
 * @tparam TYPE the type of this leaf node
 * @param actualSize the capacity of this leaf node
 * @param density the density of of this leaf node (array: 1)
 * @param node this leaf node
 * @param range the range of insert queries
 * @param findRange the range of find queries
 * @return double the time cost of this leaf node
 */
template <typename TYPE>
double CARMI::CalLeafInsertTime(int actualSize, double density,
                                const TYPE &node, const IndexPair &range,
                                const IndexPair &findRange) const {
  double time_cost = 0;
  for (int i = range.left; i < range.left + range.size; i++) {
    int predict = node.Predict(insertQuery[i].first) + findRange.left;
    int d = abs(insertQueryIndex[i] - predict);
    time_cost += (kCostBaseTime * insertQuery[i].second) / querySize;
    // add the cost of binary search between error or the entire node
    if (d <= node.error)
      time_cost +=
          (log2(node.error + 1) * insertQuery[i].second * kCostBSTime) *
          (2 - density) / querySize;
    else
      time_cost += (log2(actualSize) * insertQuery[i].second * kCostBSTime *
                    (2 - density)) /
                   querySize;
    // add the cost of moving data points
    if (density == 1)
      time_cost += kCostMoveTime * findRange.size / 2 * insertQuery[i].second /
                   querySize;
    else
      time_cost += kCostMoveTime * density / (1 - density) *
                   insertQuery[i].second / querySize;
  }
  return time_cost;
}

/**
 * @brief traverse all possible settings to find the optimal leaf node
 *
 * @param dataRange the range of data points in this node
 * @return NodeCost the optimal cost of this subtree
 */
NodeCost CARMI::DPLeaf(const DataRange &dataRange) {
  NodeCost nodeCost;
  NodeCost optimalCost = {DBL_MAX, DBL_MAX, DBL_MAX, false};
  BaseNode optimal_node_struct;

  if (kPrimaryIndex) {
    nodeCost.time = 0.0;
    nodeCost.space = 0.0;

    ExternalArray tmp;
    Train(&tmp, dataRange.initRange.left, dataRange.initRange.size);
    auto error = tmp.error;
    int findEnd = dataRange.findRange.left + dataRange.findRange.size;
    for (int i = dataRange.findRange.left; i < findEnd; i++) {
      auto predict = tmp.Predict(findQuery[i].first) + dataRange.findRange.left;
      auto d = abs(i - predict);
      nodeCost.time += (kCostBaseTime * findQuery[i].second) / querySize;
      if (d <= error)
        nodeCost.time +=
            (log2(error + 1) * findQuery[i].second * kCostBSTime) / querySize;
      else
        nodeCost.time += (log2(dataRange.initRange.size) * findQuery[i].second *
                          kCostBSTime) /
                         querySize;
    }

    nodeCost.cost = nodeCost.time + nodeCost.space * kRate;  // ns + MB * kRate
    optimalCost = {nodeCost.time, nodeCost.space, nodeCost.cost, false};
    optimal_node_struct.externalArray = tmp;

    nodeCost.isInnerNode = false;
    COST.insert({dataRange.initRange, optimalCost});
    structMap.insert({dataRange.initRange, optimal_node_struct});
    return nodeCost;
  }

  int actualSize = kThreshold;
  while (dataRange.initRange.size >= actualSize) {
    actualSize *= kExpansionScale;
  }

  if (actualSize > kLeafMaxCapacity) {
    actualSize = kLeafMaxCapacity;
  }

  // choose an array node as the leaf node
  double time_cost = 0.0;
  double space_cost = kDataPointSize * pow(2, log2(actualSize) + 1);

  ArrayType tmp(actualSize);
  Train(dataRange.initRange.left, dataRange.initRange.size, &tmp);
  time_cost +=
      CalLeafFindTime<ArrayType>(actualSize, 1, tmp, dataRange.findRange);
  time_cost += CalLeafInsertTime<ArrayType>(
      actualSize, 1, tmp, dataRange.insertRange, dataRange.findRange);

  double cost = time_cost + space_cost * kRate;  // ns + MB * kRate
  if (cost <= optimalCost.cost) {
    optimalCost = {time_cost, space_cost, cost, false};
    optimal_node_struct.array = tmp;
  }

  // choose a gapped array node as the leaf node
  float Density[5] = {0.5, 0.6, 0.7, 0.8, 0.9};  // data/capacity
  for (int i = 0; i < 5; i++) { //for
    // calculate the actual space
    int actualSize = kThreshold;
    while ((static_cast<float>(dataRange.initRange.size) /
                static_cast<float>(actualSize) >=
            Density[i]))
      actualSize = static_cast<float>(actualSize) / Density[i] + 1;
    if (actualSize > kLeafMaxCapacity) {
      actualSize = kLeafMaxCapacity;
    }

    GappedArrayType tmpNode(actualSize);
    tmpNode.density = Density[i];

    time_cost = 0.0;
    space_cost = kDataPointSize * pow(2, log2(actualSize) + 1);

    Train(dataRange.initRange.left, dataRange.initRange.size, &tmpNode);
    time_cost += CalLeafFindTime<GappedArrayType>(actualSize, Density[i],
                                                  tmpNode, dataRange.findRange);
    time_cost += CalLeafInsertTime<GappedArrayType>(
        actualSize, Density[i], tmpNode, dataRange.insertRange,
        dataRange.findRange);
    cost = time_cost + space_cost * kRate;  // ns + MB * kRate
    if (cost <= optimalCost.cost) {
      optimalCost = {time_cost, space_cost, cost, false};
      optimal_node_struct.ga = tmpNode;
    }
  }

  COST.insert({dataRange.initRange, optimalCost});
  // check ,compare
  structMap.insert({dataRange.initRange, optimal_node_struct});

  return optimalCost;
}

#endif  // SRC_CARMI_CONSTRUCT_DP_LEAF_H_
