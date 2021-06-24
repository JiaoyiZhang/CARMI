/**
 * @file dp_inner.h
 * @author Jiaoyi
 * @brief use dynamic programming algorithm to construct inner nodes
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_DP_INNER_H_
#define SRC_INCLUDE_CONSTRUCT_DP_INNER_H_
#include <float.h>

#include <algorithm>
#include <vector>

#include "../carmi.h"

template <typename KeyType, typename ValueType>
template <typename TYPE>
void CARMI<KeyType, ValueType>::ChooseBetterInner(int c,
                                                  double frequency_weight,
                                                  double time_cost,
                                                  const DataRange &dataRange,
                                                  NodeCost *optimalCost,
                                                  TYPE *optimal_node_struct) {
  double space_cost = kBaseNodeSpace * c;  // MB
  time_cost = time_cost * frequency_weight;
  double RootCost = time_cost + lambda * space_cost;
  if (RootCost > optimalCost->cost) return;

  SubDataset subDataset(c);
  auto node = InnerDivideAll<TYPE>(c, dataRange, &subDataset);

  for (int i = 0; i < c; i++) {
    NodeCost res;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size == dataRange.initRange.size) {
      return;
    }
    if (subDataset.subInit[i].size > carmi_params::kAlgorithmThreshold)
      res = GreedyAlgorithm(range);
    else
      res = DP(range);

    space_cost += res.space;
    time_cost += res.time;
    RootCost += lambda * res.space + res.time;
  }
  if (RootCost <= optimalCost->cost) {
    *optimalCost = {time_cost, space_cost, RootCost};
    *optimal_node_struct = node;
  }
}

template <typename KeyType, typename ValueType>
NodeCost CARMI<KeyType, ValueType>::DPInner(const DataRange &dataRange) {
  NodeCost nodeCost;
  NodeCost optimalCost = {DBL_MAX, DBL_MAX, DBL_MAX};
  BaseNode<KeyType> optimal_node_struct;
  optimal_node_struct = emptyNode;
  double frequency_weight = CalculateFrequencyWeight(dataRange);
  int tmpEnd = std::min(0x00FFFFFF, dataRange.initRange.size / 2);
  tmpEnd = std::max(tmpEnd, kMinChildNumber);
#ifdef DEBUG
  if (dataRange.initRange.left == 9220302 ||
      dataRange.initRange.left == 3376646) {
    std::cout << " left:" << dataRange.initRange.left
              << ",\tsize:" << dataRange.initRange.size << std::endl;
  }
#endif  // DEBUG
  for (int c = kMinChildNumber; c <= tmpEnd; c *= 2) {
    ChooseBetterInner<LRModel>(c, frequency_weight, carmi_params::kLRInnerTime,
                               dataRange, &optimalCost,
                               &(optimal_node_struct.lr));
    ChooseBetterInner<PLRModel>(c, frequency_weight,
                                carmi_params::kPLRInnerTime, dataRange,
                                &optimalCost, &(optimal_node_struct.plr));
    if (c <= kHisMaxChildNumber)
      ChooseBetterInner<HisModel>(c, frequency_weight,
                                  carmi_params::kHisInnerTime, dataRange,
                                  &optimalCost, &(optimal_node_struct.his));
    if (c <= kBSMaxChildNumber)
      ChooseBetterInner<BSModel>(c, frequency_weight,
                                 carmi_params::kBSInnerTime, dataRange,
                                 &optimalCost, &(optimal_node_struct.bs));
  }

#ifdef DEBUG
  if (optimal_node_struct.array.flagNumber == emptyNode.array.flagNumber) {
    std::cout << "optimal_node_struct is equal to the emptyNode, left:"
              << dataRange.initRange.left
              << ",\tsize:" << dataRange.initRange.size << std::endl;
  }
#endif  // DEBUG

  structMap.insert({dataRange.initRange, optimal_node_struct});
  COST.insert({dataRange.initRange, optimalCost});
  nodeCost = {optimalCost.time, optimalCost.space, optimalCost.cost};
  return nodeCost;
}

#endif  // SRC_INCLUDE_CONSTRUCT_DP_INNER_H_
