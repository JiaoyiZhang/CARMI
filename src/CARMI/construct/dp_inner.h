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
 * @param optimalCost the optimal cost of the previous setting
 * @param optimalStruct the optimal setting
 * @param time_cost the time cost of this inner node
 * @param frequency the frequency of data points
 * @param c the child number of this inner node
 * @param type the type of this inne node
 * @param dataRange the range of data points in this node
 */
template <typename TYPE>
void CARMI::CalInner(int c, NodeType type, int frequency, double time_cost,
                     const DataRange &dataRange, NodeCost *optimalCost,
                     ParamStruct *optimalStruct) {
  double space_cost = BaseNodeSpace * c;  // MB
  time_cost = time_cost * frequency / querySize;
  double RootCost = time_cost + kRate * space_cost;
  space_cost *= kRate;
  if (RootCost > optimalCost->cost) return;

  SubDataset subDataset(c);
  InnerDivideAll<TYPE>(c, dataRange, &subDataset);

  std::vector<MapKey> tmpChild;
  for (int i = 0; i < c; i++) {
    NodeCost res;
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
    space_cost += res.space;
    time_cost += res.time;
    RootCost += res.space + res.time;
  }
  if (RootCost <= optimalCost->cost) {
    *optimalCost = {time_cost, space_cost, RootCost, true};
    *optimalStruct = ParamStruct(type, c, kDensity, tmpChild);
  }
}

/**
 * @brief traverse all possible settings to find the optimal inner node
 * @param dataRange the range of data points in this node
 * @return the optimal cost of this subtree
 */
NodeCost CARMI::dpInner(const DataRange &dataRange) {
  NodeCost nodeCost;
  NodeCost optimalCost = NodeCost{DBL_MAX, DBL_MAX, DBL_MAX, true};
  ParamStruct optimalStruct = {0, 32, 2, std::vector<MapKey>()};
  int frequency = 0;
  int findEnd = dataRange.findRange.left + dataRange.findRange.size;
  for (int l = dataRange.findRange.left; l < findEnd; l++)
    frequency += findQuery[l].second;
  int insertEnd = dataRange.insertRange.left + dataRange.insertRange.size;
  for (int l = dataRange.insertRange.left; l < insertEnd; l++)
    frequency += insertQuery[l].second;
  int tmpEnd = dataRange.initRange.size / 2;
  for (int c = 2; c < tmpEnd; c *= 2) {
#ifdef DEBUG
    if (c * 512 < dataRange.initRange.size) continue;
#endif  // DEBUG
    CalInner<LRModel>(c, LR_INNER_NODE, frequency, LRInnerTime, dataRange,
                      &optimalCost, &optimalStruct);
    CalInner<PLRModel>(c, PLR_INNER_NODE, frequency, PLRInnerTime, dataRange,
                       &optimalCost, &optimalStruct);
    if (c <= 160)
      CalInner<HisModel>(c, HIS_INNER_NODE, frequency, HisInnerTime, dataRange,
                         &optimalCost, &optimalStruct);
    if (c <= 20)
      CalInner<BSModel>(c, BS_INNER_NODE, frequency, BSInnerTime, dataRange,
                        &optimalCost, &optimalStruct);
  }
  MapKey key = {true, {dataRange.initRange.left, dataRange.initRange.size}};
  if (optimalCost.time < DBL_MAX) structMap.insert({key, optimalStruct});
  nodeCost = {optimalCost.time, optimalCost.space, optimalCost.cost, true};
  return nodeCost;
}

#endif  // SRC_CARMI_CONSTRUCT_DP_INNER_H_
