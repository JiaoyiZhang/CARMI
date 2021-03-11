/**
 * @file greedy.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_CONSTRUCT_GREEDY_H_
#define SRC_CARMI_CONSTRUCT_GREEDY_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../../params.h"
#include "../carmi.h"
#include "./dp_inner.h"
#include "./minor_function.h"
#include "./structures.h"

template <typename TYPE>
void CARMI::CheckGreedy(int c, NodeType type, double pi, int frequency,
                        double time_cost, const IndexPair &range,
                        ParamStruct *optimalStruct, NodeCost *optimalCost) {
  std::vector<IndexPair> perSize(c, emptyRange);
  double space_cost = 64.0 * c / 1024 / 1024;

  TYPE node = TYPE();
  node.SetChildNumber(c);
  Train(range.left, range.size, initDataset, &node);

  NodePartition<TYPE>(node, range, initDataset, &perSize);
  double entropy = CalculateEntropy(range.size, c, perSize);
  double cost =
      (time_cost + static_cast<float>(kRate * space_cost) / pi) / entropy;

  if (cost <= optimalCost->cost) {
    optimalStruct->type = type;
    optimalStruct->childNum = c;
    *optimalCost = {time_cost * frequency / querySize, kRate * space_cost, cost,
                    true};
  }
}
NodeCost CARMI::GreedyAlgorithm(const DataRange &dataRange) {
  NodeCost nodeCost = {0, 0, 0, false};
  if (dataRange.initRange.size == 0 && dataRange.findRange.size == 0)
    return nodeCost;

  NodeCost optimalCost = NodeCost{DBL_MAX, DBL_MAX, DBL_MAX, true};
  double pi = static_cast<float>(dataRange.findRange.size +
                                 dataRange.insertRange.size) /
              querySize;
  ParamStruct optimalStruct = {LR_INNER_NODE, 32, 2, std::vector<MapKey>()};
  int frequency = 0;
  int findEnd = dataRange.findRange.left + dataRange.findRange.size;
  for (int l = dataRange.findRange.left; l < findEnd; l++)
    frequency += findQuery[l].second;
  int insertEnd = dataRange.insertRange.left + dataRange.insertRange.size;
  for (int l = dataRange.insertRange.left; l < insertEnd; l++)
    frequency += insertQuery[l].second;
  int tmpEnd = dataRange.initRange.size / 2;
  IndexPair singleRange(dataRange.initRange.left, dataRange.initRange.size);
  for (int c = 2; c < tmpEnd; c *= 2) {
#ifdef DEBUG
    if (c * 512 < dataRange.initRange.size) continue;
#endif  // DEBUG
    CheckGreedy<LRModel>(c, LR_INNER_NODE, pi, frequency, LRInnerTime,
                         dataRange.initRange, &optimalStruct, &optimalCost);
    CheckGreedy<LRModel>(c, PLR_INNER_NODE, pi, frequency, PLRInnerTime,
                         dataRange.initRange, &optimalStruct, &optimalCost);
    CheckGreedy<HisModel>(c, HIS_INNER_NODE, pi, frequency, HisInnerTime,
                          dataRange.initRange, &optimalStruct, &optimalCost);
    CheckGreedy<BSModel>(c, BS_INNER_NODE, pi, frequency, BSInnerTime,
                         dataRange.initRange, &optimalStruct, &optimalCost);
  }

  // construct child
  SubDataset subDataset = SubDataset(optimalStruct.childNum);
  switch (optimalStruct.type) {
    case LR_INNER_NODE: {
      InnerDivideAll<LRModel>(optimalStruct.childNum, dataRange, &subDataset);
      break;
    }
    case PLR_INNER_NODE: {
      InnerDivideAll<PLRModel>(optimalStruct.childNum, dataRange, &subDataset);
      break;
    }
    case HIS_INNER_NODE: {
      InnerDivideAll<HisModel>(optimalStruct.childNum, dataRange, &subDataset);
      break;
    }
    case BS_INNER_NODE: {
      InnerDivideAll<BSModel>(optimalStruct.childNum, dataRange, &subDataset);
      break;
    }
  }

  std::vector<MapKey> tmpChild;
  for (int i = 0; i < optimalStruct.childNum; i++) {
    NodeCost res = {0, 0, 0, true};
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size + subDataset.subInsert[i].size >
        kAlgorithmThreshold)
      res = GreedyAlgorithm(range);
    else
      res = dp(range);

    MapKey key = {res.isInnerNode,
                  {subDataset.subInit[i].size, subDataset.subInsert[i].size}};
    tmpChild.push_back(key);
  }

  MapKey nodeKey = {true, {dataRange.initRange.left, dataRange.initRange.size}};
  optimalStruct.child = tmpChild;
  if (optimalCost.time < DBL_MAX) structMap.insert({nodeKey, optimalStruct});
  nodeCost = {optimalCost.time, optimalCost.space, optimalCost.cost, true};
  return nodeCost;
}
#endif  // SRC_CARMI_CONSTRUCT_GREEDY_H_
