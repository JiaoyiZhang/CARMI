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
#ifndef CONSTRUCT_DP_INNER_H_
#define CONSTRUCT_DP_INNER_H_
#include <float.h>

#include <algorithm>
#include <vector>

#include "../carmi.h"

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
template <typename InnerNodeType>
void CARMI<KeyType, ValueType, Compare, Alloc>::UpdateDPOptSetting(
    const DataRange &dataRange, int c, double frequency_weight,
    NodeCost *optimalCost, InnerNodeType *optimal_node_struct) {
  double space_cost = kBaseNodeSpace * static_cast<double>(c);
  double time_cost =
      InnerNodeType::kTimeCost * static_cast<double>(frequency_weight);
  double RootCost = time_cost + lambda * space_cost;
  // Case 1: the cost of the root node has been larger than the optimal cost,
  // return directly
  if (RootCost > optimalCost->cost) {
    return;
  }

  // Case 2: construct an inner node and divide the dataset into c sub-datasets
  SubDataset subDataset(c);
  auto currnode = InnerDivideAll<InnerNodeType>(dataRange, c, &subDataset);

  for (int i = 0; i < c; i++) {
    // calculate the cost of each child node
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    // Case 2.1: if this inner node fails to divide dataset evenly, return
    // directly
    if (range.initRange.size + range.initRange.size ==
        dataRange.initRange.size + dataRange.insertRange.size) {
      return;
    }

    NodeCost res = DP(range);

    space_cost += res.space;
    time_cost += res.time;
    RootCost += lambda * res.space + res.time;
  }
  // if the current cost is smaller than the optimal cost, update the optimal
  // cost and node setting
  if (RootCost <= optimalCost->cost) {
    *optimalCost = {time_cost, space_cost, RootCost};
    *optimal_node_struct = currnode;
  }
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
NodeCost CARMI<KeyType, ValueType, Compare, Alloc>::DPInner(
    const DataRange &dataRange) {
  // the optimal cost of this sub-dataset
  NodeCost optimalCost{DBL_MAX, DBL_MAX, DBL_MAX};
  // the optimal node of this sub-dataset
  BaseNode<KeyType, ValueType, Compare, Alloc> optimal_node_struct = emptyNode;
  // calculate the weight of the frequency of this sub-dataset (findQuery and
  // insertQury)
  double frequency_weight = CalculateFrequencyWeight(dataRange);
  int tmpEnd = std::min(0x00FFFFFF, dataRange.initRange.size / 16);
  tmpEnd = std::max(tmpEnd, kMinChildNumber);
  for (int c = kMinChildNumber; c <= tmpEnd; c *= 2) {
    // Case 1: construct a LR inner node, if it is better than the current
    // optimal setting, then use it to update the optimal setting
    UpdateDPOptSetting<LRModel<KeyType, ValueType>>(
        dataRange, c, frequency_weight, &optimalCost,
        &(optimal_node_struct.lr));
    // Case 2: construct a P. LR inner node, if it is better than the current
    // optimal setting, then use it to update the optimal setting
    UpdateDPOptSetting<PLRModel<KeyType, ValueType>>(
        dataRange, c, frequency_weight, &optimalCost,
        &(optimal_node_struct.plr));
    // Case 3: construct a His inner node, if it is better than the current
    // optimal setting, then use it to update the optimal setting
    if (c <= kHisMaxChildNumber)
      UpdateDPOptSetting<HisModel<KeyType, ValueType>>(
          dataRange, c, frequency_weight, &optimalCost,
          &(optimal_node_struct.his));
    // Case 4: construct a BS inner node, if it is better than the current
    // optimal setting, then use it to update the optimal setting
    if (c <= kBSMaxChildNumber)
      UpdateDPOptSetting<BSModel<KeyType, ValueType>>(
          dataRange, c, frequency_weight, &optimalCost,
          &(optimal_node_struct.bs));
  }
  // store the optimal setting of this sub-dataset
  structMap.insert({dataRange.initRange, optimal_node_struct});
  // store the minimum cost of this sub-dataset
  COST.insert({dataRange.initRange, optimalCost});
  return optimalCost;
}

#endif  // CONSTRUCT_DP_INNER_H_
