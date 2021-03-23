/**
 * @file dp_inner.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_CONSTRUCT_DP_INNER_H_
#define SRC_CARMI_CONSTRUCT_DP_INNER_H_
#include <float.h>

#include <vector>

#include "../carmi.h"

/**
 * @brief determine whether the current inner node's setting is
 *        better than the previous optimal setting
 *
 * @tparam TYPE the type of this inner node
 * @param c the child number of this inner node
 * @param type the type of this inne node
 * @param frequency_weight the frequency weight of these queries
 * @param time_cost the time cost of this inner node
 * @param dataRange the range of data points in this node
 * @param optimalCost the optimal cost of the previous setting
 * @param optimal_node_struct the optimal setting
 */
template <typename TYPE>
void CARMI::ChooseBetterInner(int c, NodeType type, double frequency_weight,
                              double time_cost, const DataRange &dataRange,
                              NodeCost *optimalCost,
                              TYPE *optimal_node_struct) {
  double space_cost = kBaseNodeSpace * c;  // MB
  time_cost = time_cost * frequency_weight;
  double RootCost = time_cost + kRate * space_cost;
  if (RootCost > optimalCost->cost) return;

  SubDataset subDataset(c);
  auto node = InnerDivideAll<TYPE>(c, dataRange, &subDataset);

  for (int i = 0; i < c; i++) {
    NodeCost res;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size > kAlgorithmThreshold)
      res = GreedyAlgorithm(range);
    else
      res = DP(range);

    space_cost += res.space;
    time_cost += res.time;
    RootCost += kRate * res.space + res.time;
  }
  if (RootCost <= optimalCost->cost) {
    *optimalCost = {time_cost, space_cost, RootCost};
    *optimal_node_struct = node;
  }
}

/**
 * @brief traverse all possible settings to find the optimal inner node
 *
 * @param dataRange the range of data points in this node
 * @return NodeCost the optimal cost of this subtree
 */
NodeCost CARMI::DPInner(const DataRange &dataRange) {
  NodeCost nodeCost;
  NodeCost optimalCost = {DBL_MAX, DBL_MAX, DBL_MAX};
  BaseNode optimal_node_struct = emptyNode;
  double frequency_weight = CalculateFrequencyWeight(dataRange);
  int tmpEnd = dataRange.initRange.size / 2;
  for (int c = kMinChildNumber; c < tmpEnd; c *= 2) {
#ifdef DEBUG
    if (c * 512 < dataRange.initRange.size) continue;
#endif  // DEBUG
    ChooseBetterInner<LRModel>(c, LR_INNER_NODE, frequency_weight, kLRInnerTime,
                               dataRange, &optimalCost,
                               &(optimal_node_struct.lr));
    ChooseBetterInner<PLRModel>(c, PLR_INNER_NODE, frequency_weight,
                                kPLRInnerTime, dataRange, &optimalCost,
                                &(optimal_node_struct.plr));
    if (c <= kHisMaxChildNumber)
      ChooseBetterInner<HisModel>(c, HIS_INNER_NODE, frequency_weight,
                                  kHisInnerTime, dataRange, &optimalCost,
                                  &(optimal_node_struct.his));
    if (c <= kBSMaxChildNumber)
      ChooseBetterInner<BSModel>(c, BS_INNER_NODE, frequency_weight,
                                 kBSInnerTime, dataRange, &optimalCost,
                                 &(optimal_node_struct.bs));
  }

#ifdef DEBUG
  if (optimalCost.time == DBL_MAX)
    std::cout << "wrong, dp inner time is DBL_MAX" << std::endl;
#endif  // DEBUG

  structMap.insert({dataRange.initRange, optimal_node_struct});
  COST.insert({dataRange.initRange, optimalCost});
  nodeCost = {optimalCost.time, optimalCost.space, optimalCost.cost};
  return nodeCost;
}

#endif  // SRC_CARMI_CONSTRUCT_DP_INNER_H_
