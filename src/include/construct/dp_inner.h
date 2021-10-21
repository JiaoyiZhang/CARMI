/**
 * @file dp_inner.h
 * @author Jiaoyi
 * @brief use dynamic programming algorithm to construct inner nodes
 * @version 3.0
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
template <typename InnerNodeType>
void CARMI<KeyType, ValueType>::UpdateDPOptSetting(
    int c, double frequency_weight, const DataRange &dataRange,
    NodeCost *optimalCost, InnerNodeType *optimal_node_struct) {
  double space_cost = kBaseNodeSpace * c;  // MB
  double time_cost = InnerNodeType::kTimeCost;
  time_cost = time_cost * frequency_weight;
  double RootCost = time_cost + lambda * space_cost;
  if (RootCost > optimalCost->cost) return;

  SubDataset subDataset(c);
  auto currnode = InnerDivideAll<InnerNodeType>(c, dataRange, &subDataset);

  for (int i = 0; i < c; i++) {
    NodeCost res;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size + subDataset.subInsert[i].size ==
        dataRange.initRange.size + dataRange.insertRange.size) {
      return;
    }
    if (subDataset.subInit[i].size + subDataset.subInsert[i].size >
        carmi_params::kAlgorithmThreshold)
      res = GreedyAlgorithm(range);
    else
      res = DP(range);

    space_cost += res.space;
    time_cost += res.time;
    RootCost += lambda * res.space + res.time;
  }
  if (RootCost <= optimalCost->cost) {
    *optimalCost = {time_cost, space_cost, RootCost};
    *optimal_node_struct = currnode;
  }
}

template <typename KeyType, typename ValueType>
NodeCost CARMI<KeyType, ValueType>::DPInner(const DataRange &dataRange) {
  NodeCost nodeCost;
  NodeCost optimalCost = {DBL_MAX, DBL_MAX, DBL_MAX};
  BaseNode<KeyType, ValueType> optimal_node_struct;
  optimal_node_struct = emptyNode;
  double frequency_weight = CalculateFrequencyWeight(dataRange);
  int tmpEnd = std::min(0x00FFFFFF, dataRange.initRange.size / 4);
  tmpEnd = std::max(tmpEnd, kMinChildNumber);
  for (int c = kMinChildNumber; c <= tmpEnd; c *= 2) {
    UpdateDPOptSetting<LRModel<KeyType, ValueType>>(c, frequency_weight,
                                                    dataRange, &optimalCost,
                                                    &(optimal_node_struct.lr));
    UpdateDPOptSetting<PLRModel<KeyType, ValueType>>(
        c, frequency_weight, dataRange, &optimalCost,
        &(optimal_node_struct.plr));
    if (c <= kHisMaxChildNumber)
      UpdateDPOptSetting<HisModel<KeyType, ValueType>>(
          c, frequency_weight, dataRange, &optimalCost,
          &(optimal_node_struct.his));
    if (c <= kBSMaxChildNumber)
      UpdateDPOptSetting<BSModel<KeyType, ValueType>>(
          c, frequency_weight, dataRange, &optimalCost,
          &(optimal_node_struct.bs));
  }

  structMap.insert({dataRange.initRange, optimal_node_struct});
  COST.insert({dataRange.initRange, optimalCost});
  nodeCost = {optimalCost.time, optimalCost.space, optimalCost.cost};
  return nodeCost;
}

#endif  // SRC_INCLUDE_CONSTRUCT_DP_INNER_H_
