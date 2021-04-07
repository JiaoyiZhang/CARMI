/**
 * @file greedy.h
 * @author Jiaoyi
 * @brief use greedy algorithm to construct inner nodes
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_GREEDY_H_
#define SRC_INCLUDE_CONSTRUCT_GREEDY_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../../params.h"
#include "../carmi.h"
#include "./dp_inner.h"
#include "./minor_function.h"
#include "./structures.h"
/**
 * @brief choose the optimal setting
 *
 * @tparam TYPE the type of this node
 * @param c the number of child nodes
 * @param type the type index of this node
 * @param frequency_weight the frequency weight of these queries
 * @param time_cost the time cost of this node
 * @param range the range of queries
 * @param optimal_node_struct the optimal setting
 * @param optimalCost the optimal cost
 */
template <typename KeyType, typename ValueType>
template <typename TYPE>
void CARMI<KeyType, ValueType>::IsBetterGreedy(
    int c, NodeType type, double frequency_weight, double time_cost,
    const IndexPair &range, TYPE *optimal_node_struct, NodeCost *optimalCost) {
  std::vector<IndexPair> perSize(c, emptyRange);
  double space_cost = carmi_params::kBaseNodeSpace * c;

  TYPE node;
  node.SetChildNumber(c);
  Train(range.left, range.size, initDataset, &node);

  NodePartition<TYPE>(node, range, initDataset, &perSize);
  double entropy = CalculateEntropy(range.size, c, perSize);
  double cost = (time_cost + kRate * space_cost / frequency_weight) / entropy;

  if (cost <= optimalCost->cost) {
    *optimal_node_struct = node;
    *optimalCost = {time_cost, space_cost, cost};
  }
}
/**
 * @brief the greedy algorithm
 *
 * @param dataRange the range of these queries
 * @return NodeCost the optimal cost of the subtree
 */
template <typename KeyType, typename ValueType>
NodeCost CARMI<KeyType, ValueType>::GreedyAlgorithm(
    const DataRange &dataRange) {
  NodeCost nodeCost = emptyCost;
  if (dataRange.initRange.size == 0) {
    nodeCost = emptyCost;
    ConstructEmptyNode(dataRange);
    return nodeCost;
  }

  NodeCost optimalCost = {DBL_MAX, DBL_MAX, DBL_MAX};
  BaseNode optimal_node_struct;
  double frequency_weight = CalculateFrequencyWeight(dataRange);
  int tmpEnd = dataRange.initRange.size / 2;
  IndexPair singleRange(dataRange.initRange.left, dataRange.initRange.size);
  for (int c = carmi_params::kMinChildNumber; c < tmpEnd; c *= 2) {
    IsBetterGreedy<LRModel>(c, LR_INNER_NODE, frequency_weight,
                            carmi_params::kLRInnerTime, dataRange.initRange,
                            &(optimal_node_struct.lr), &optimalCost);
    IsBetterGreedy<PLRModel>(c, PLR_INNER_NODE, frequency_weight,
                             carmi_params::kPLRInnerTime, dataRange.initRange,
                             &(optimal_node_struct.plr), &optimalCost);
    IsBetterGreedy<HisModel>(c, HIS_INNER_NODE, frequency_weight,
                             carmi_params::kHisInnerTime, dataRange.initRange,
                             &(optimal_node_struct.his), &optimalCost);
    IsBetterGreedy<BSModel>(c, BS_INNER_NODE, frequency_weight,
                            carmi_params::kBSInnerTime, dataRange.initRange,
                            &(optimal_node_struct.bs), &optimalCost);
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
    NodeCost res = emptyCost;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size > kAlgorithmThreshold)
      res = GreedyAlgorithm(range);
    else
      res = DP(range);
  }

  structMap.insert({dataRange.initRange, optimal_node_struct});
  COST.insert({dataRange.initRange, optimalCost});
  nodeCost = {optimalCost.time, optimalCost.space, optimalCost.cost};
  return nodeCost;
}
#endif  // SRC_INCLUDE_CONSTRUCT_GREEDY_H_
