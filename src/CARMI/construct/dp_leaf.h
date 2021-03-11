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
#ifndef DP_LEAF_H
#define DP_LEAF_H

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../../params.h"
#include "../func/inlineFunction.h"
#include "structures.h"
using namespace std;

/**
 * @brief calculate the time cost of find queries
 * @param node this leaf node
 * @param range the range of find queries
 * @param actualSize the capacity of this leaf node
 * @param density the density of of this leaf node (array: 1)
 * @return the time cost of this leaf node
 */
template <typename TYPE>
double CARMI::CalLeafFindTime(int actualSize, double density, const TYPE &node,
                              const DataRange &range) const {
  double time = 0;
  for (int i = range.left; i < range.left + range.size; i++) {
    auto predict = node.Predict(findQuery[i].first) + range.left;
    auto d = abs(i - predict);
    time += (CostBaseTime * findQuery[i].second) / querySize;
    if (d <= node.error) {
      time +=
          (log(node.error + 1) / log(2) * findQuery[i].second * CostBSTime) *
          (2 - density) / querySize;
    } else
      time += (log(actualSize) / log(2) * findQuery[i].second * CostBSTime *
               (2 - density)) /
              querySize;
  }
  return time;
}

/**
 * @brief calculate the time cost of insert queries
 * @param node this leaf node
 * @param range the range of insert queries
 * @param findRange the range of find queries
 * @param actualSize the capacity of this leaf node
 * @param density the density of of this leaf node (array: 1)
 * @return the time cost of this leaf node
 */
template <typename TYPE>
double CARMI::CalLeafInsertTime(int actualSize, double density,
                                const TYPE &node, const DataRange &range,
                                const DataRange &findRange) const {
  double time = 0;
  for (int i = range.left; i < range.left + range.size; i++) {
    int predict = node.Predict(insertQuery[i].first) + findRange.left;
    int d = abs(i - predict);
    time += (CostBaseTime * insertQuery[i].second) / querySize;
    int actual;
    if (density == 1) {
      actual = TestBinarySearch(insertQuery[i].first, findRange.left,
                                findRange.left + findRange.size);
      time +=
          CostMoveTime * findRange.size / 2 * insertQuery[i].second / querySize;
    } else {
      actual = TestBinarySearch(insertQuery[i].first, findRange.left,
                                findRange.left + findRange.size);
      time += CostMoveTime * density / (1 - density) * insertQuery[i].second /
              querySize;
    }
    d = abs(actual - predict);

    if (d <= node.error) {
      time +=
          (log(node.error + 1) / log(2) * insertQuery[i].second * CostBSTime) *
          (2 - density) / querySize;
    } else
      time += (log(actualSize) / log(2) * insertQuery[i].second * CostBSTime *
               (2 - density)) /
              querySize;
  }
  return time;
}

/**
 * @brief traverse all possible settings to find the optimal leaf node
 * @param dataRange the range of data points in this node
 * @return the optimal cost of this subtree
 */
NodeCost CARMI::dpLeaf(const IndexPair &dataRange) {
  NodeCost nodeCost;
  auto it = COST.find(dataRange.initRange);
  if (it != COST.end()) {
    nodeCost = it->second;
    nodeCost.isInnerNode = false;
    return nodeCost;
  }

  NodeCost optimalCost = NodeCost{DBL_MAX, DBL_MAX, DBL_MAX, false};
  ParamStruct optimalStruct;

  if (kPrimaryIndex) {
    nodeCost.time = 0.0;
    nodeCost.space = 0.0;

    auto tmp = YCSBLeaf();
    Train(&tmp, dataRange.initRange.left, dataRange.initRange.size);
    auto error =
        UpdateError(&tmp, dataRange.initRange.left, dataRange.initRange.size);
    int findEnd = dataRange.findRange.left + dataRange.findRange.size;
    for (int i = dataRange.findRange.left; i < findEnd; i++) {
      auto predict = tmp.Predict(findQuery[i].first) + dataRange.findRange.left;
      auto d = abs(i - predict);
      nodeCost.time += (CostBaseTime * findQuery[i].second) / querySize;
      if (d <= error) {
        nodeCost.time +=
            (log(error + 1) / log(2) * findQuery[i].second * CostBSTime) /
            querySize;
      } else
        nodeCost.time += (log(dataRange.initRange.size) / log(2) *
                          findQuery[i].second * CostBSTime) /
                         querySize;
    }

    int insertEnd = dataRange.insertRange.left + dataRange.insertRange.size;
    for (int i = dataRange.insertRange.left; i < insertEnd; i++) {
      nodeCost.time +=
          ((CostBaseTime + CostMoveTime) * insertQuery[i].second) / querySize;
    }

    nodeCost.cost = nodeCost.time + nodeCost.space * kRate;  // ns + MB * kRate
    optimalCost = {nodeCost.time, nodeCost.space * kRate, nodeCost.cost, false};
    optimalStruct = ParamStruct(6, 0, 2, vector<MapKey>());

    nodeCost.space *= kRate;
    nodeCost.isInnerNode = false;
    MapKey key = {false, {dataRange.initRange.left, dataRange.initRange.size}};
    COST.insert({dataRange.initRange, optimalCost});
    structMap.insert({key, optimalStruct});
    return nodeCost;
  }

  int actualSize = kThreshold;
  while (dataRange.initRange.size >= actualSize) actualSize *= kExpansionScale;

  if (actualSize > 4096) actualSize = 4096;

  // choose an array node as the leaf node
  double time = 0.0;
  double space = 16.0 * pow(2, log(actualSize) / log(2) + 1) / 1024 / 1024;

  auto tmp = ArrayType(actualSize);
  Train(&tmp, dataRange.initRange.left, dataRange.initRange.size);
  auto error =
      UpdateError(&tmp, dataRange.initRange.left, dataRange.initRange.size);
  CalLeafFindTime<ArrayType>(actualSize, 1, tmp, dataRange.findRange);
  CalLeafInsertTime<ArrayType>(actualSize, 1, tmp, dataRange.insertRange,
                               dataRange.findRange);

  double cost = time + space * kRate;  // ns + MB * kRate
  if (cost <= optimalCost.cost) {
    optimalCost = NodeCost{time, space * kRate, cost, false};
    optimalStruct = ParamStruct(4, 0, 1, vector<MapKey>());
  }

  // choose a gapped array node as the leaf node
  float Density[3] = {0.5, 0.7, 0.8};  // data/capacity
  for (int i = 0; i < 3; i++) {
    // calculate the actual space
    int actualSize = kThreshold;
    while ((float(dataRange.initRange.size) / float(actualSize) >= Density[i]))
      actualSize = float(actualSize) / Density[i] + 1;
    if (actualSize > 4096) actualSize = 4096;

    auto tmpNode = GappedArrayType(actualSize);
    tmpNode.density = Density[i];

    time = 0.0;
    space = 16.0 * pow(2, log(actualSize) / log(2) + 1) / 1024 / 1024;

    Train(&tmpNode, dataRange.initRange.left, dataRange.initRange.size);
    auto errorGA = UpdateError(&tmpNode, dataRange.initRange.left,
                               dataRange.initRange.size);
    CalLeafFindTime<GappedArrayType>(actualSize, Density[i], tmpNode,
                                     dataRange.findRange);
    CalLeafInsertTime<GappedArrayType>(actualSize, Density[i], tmpNode,
                                       dataRange.insertRange,
                                       dataRange.findRange);
    cost = time + space * kRate;  // ns + MB * kRate
    if (cost <= optimalCost.cost) {
      optimalCost = NodeCost{time, space * kRate, cost, false};
      optimalStruct = ParamStruct(5, 0, Density[i], vector<MapKey>());
    }
  }

  MapKey key = {false, {dataRange.initRange.left, dataRange.initRange.size}};
  COST.insert({dataRange.initRange, optimalCost});
  structMap.insert({key, optimalStruct});

  return optimalCost;
}

#endif  // !DP_LEAF_H