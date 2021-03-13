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
void CARMI::CheckGreedy(int c, NodeType type, double pi,
                        double frequency_weight, double time_cost,
                        const IndexPair &range, BaseNode *optimal_node_struct,
                        NodeCost *optimalCost) {
  std::vector<IndexPair> perSize(c, IndexPair(-1, 0));
  double space_cost = BaseNodeSpace * c;

  TYPE node;
  node.SetChildNumber(c);
  Train(range.left, range.size, &node);

  NodePartition<TYPE>(node, range, initDataset, &perSize);
  double entropy = CalculateEntropy(range.size, c, perSize);
  double cost =
      (time_cost + static_cast<float>(kRate * space_cost) / pi) / entropy;

  if (cost <= optimalCost->cost) {
    (*optimal_node_struct).lr = *(reinterpret_cast<LRModel *>(&node));
    *optimalCost = {time_cost, space_cost, cost, true};
  }
}

NodeCost CARMI::GreedyAlgorithm(const DataRange &dataRange) {
  NodeCost nodeCost = {0, 0, 0, false};
  if (dataRange.initRange.size == 0 && dataRange.findRange.size == 0)
    return nodeCost;

  NodeCost optimalCost = {DBL_MAX, DBL_MAX, DBL_MAX, true};
  double pi = static_cast<float>(dataRange.findRange.size +
                                 dataRange.insertRange.size) /
              querySize;
  BaseNode optimal_node_struct;
  int frequency = 0;
  int findEnd = dataRange.findRange.left + dataRange.findRange.size;
  for (int l = dataRange.findRange.left; l < findEnd; l++)
    frequency += findQuery[l].second;
  int insertEnd = dataRange.insertRange.left + dataRange.insertRange.size;
  for (int l = dataRange.insertRange.left; l < insertEnd; l++)
    frequency += insertQuery[l].second;
  double frequency_weight = static_cast<double>(frequency) / querySize;
  int tmpEnd = dataRange.initRange.size / 2;
  IndexPair singleRange(dataRange.initRange.left, dataRange.initRange.size);
  for (int c = 2; c < tmpEnd; c *= 2) {
#ifdef DEBUG
    if (c * 512 < dataRange.initRange.size) continue;
#endif  // DEBUG
    CheckGreedy<LRModel>(c, LR_INNER_NODE, pi, frequency_weight, LRInnerTime,
                         dataRange.initRange, &optimal_node_struct,
                         &optimalCost);
    CheckGreedy<LRModel>(c, PLR_INNER_NODE, pi, frequency_weight, PLRInnerTime,
                         dataRange.initRange, &optimal_node_struct,
                         &optimalCost);
    CheckGreedy<HisModel>(c, HIS_INNER_NODE, pi, frequency_weight, HisInnerTime,
                          dataRange.initRange, &optimal_node_struct,
                          &optimalCost);
    CheckGreedy<BSModel>(c, BS_INNER_NODE, pi, frequency_weight, BSInnerTime,
                         dataRange.initRange, &optimal_node_struct,
                         &optimalCost);
  }

  // construct child
  int childNum = optimal_node_struct.lr.flagNumber & 0x00FFFFFF;
  int type = optimal_node_struct.lr.flagNumber >> 24;
  SubDataset subDataset(childNum);
  switch (type) {
    case LR_INNER_NODE: {
      InnerDivideAll<LRModel>(childNum, dataRange, &subDataset);
      break;
    }
    case PLR_INNER_NODE: {
      InnerDivideAll<PLRModel>(childNum, dataRange, &subDataset);
      break;
    }
    case HIS_INNER_NODE: {
      InnerDivideAll<HisModel>(childNum, dataRange, &subDataset);
      break;
    }
    case BS_INNER_NODE: {
      InnerDivideAll<BSModel>(childNum, dataRange, &subDataset);
      break;
    }
  }

  for (int i = 0; i < childNum; i++) {
    NodeCost res = {0, 0, 0, true};
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size > kAlgorithmThreshold)
      res = GreedyAlgorithm(range);
    else
      res = dp(range);
  }

  if (optimalCost.time < DBL_MAX)
    structMap.insert({dataRange.initRange, optimal_node_struct});
  nodeCost = {optimalCost.time, optimalCost.space, optimalCost.cost, true};
  return nodeCost;
}
#endif  // SRC_CARMI_CONSTRUCT_GREEDY_H_
