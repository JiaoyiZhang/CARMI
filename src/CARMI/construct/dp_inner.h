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
#ifndef DP_INNER_H
#define DP_INNER_H
#include <float.h>

#include "../carmi.h"
using namespace std;

/**
 * @brief determine whether the current inner node's setting is
 *        better than the previous optimal setting
 * @param optimalCost the optimal cost of the previous setting
 * @param optimalStruct the optimal setting
 * @param time the time cost of this inner node
 * @param frequency the frequency of these data points
 * @param c the child number of this inner node
 * @param type the type of this inne node
 * @param dataRange the range of data points in this node
 */
template <typename TYPE>
void CARMI::CalInner(int c, int type, int frequency, double time,
                     const IndexPair &dataRange, NodeCost *optimalCost,
                     ParamStruct *optimalStruct) {
  double space = 64.0 * c / 1024 / 1024;  // MB
  time = time * frequency / querySize;
  double RootCost = time + kRate * space;
  space *= kRate;
  if (RootCost > optimalCost->cost) return;

  SubDataset subDataset(c);
  InnerDivideAll<TYPE>(c, dataRange, &subDataset);

  vector<MapKey> tmpChild;
  for (int i = 0; i < c; i++) {
    NodeCost res;
    IndexPair range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size + subDataset.subInsert[i].size >
        kAlgorithmThreshold)
      res = GreedyAlgorithm(range);
    else
      res = dp(range);

    MapKey key = {res.isInnerNode,
                  {subDataset.subInit[i].size, subDataset.subInsert[i].size}};
    tmpChild.push_back(key);
    space += res.space;
    time += res.time;
    RootCost += res.space + res.time;
  }
  if (RootCost <= optimalCost->cost) {
    *optimalCost = {time, space, RootCost, true};
    *optimalStruct = ParamStruct(type, c, kDensity, tmpChild);
  }
}

/**
 * @brief traverse all possible settings to find the optimal inner node
 * @param dataRange the range of data points in this node
 * @return the optimal cost of this subtree
 */
NodeCost CARMI::dpInner(const IndexPair &dataRange) {
  NodeCost nodeCost;
  NodeCost optimalCost = NodeCost{DBL_MAX, DBL_MAX, DBL_MAX, true};
  double space;
  ParamStruct optimalStruct = {0, 32, 2, vector<MapKey>()};
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
    CalInner<LRModel>(c, 0, frequency, LRInnerTime, dataRange, &optimalCost,
                      &optimalStruct);
    CalInner<PLRModel>(c, 1, frequency, PLRInnerTime, dataRange, &optimalCost,
                       &optimalStruct);
    if (c <= 160)
      CalInner<HisModel>(c, 2, frequency, HisInnerTime, dataRange, &optimalCost,
                         &optimalStruct);
    if (c <= 20)
      CalInner<BSModel>(c, 3, frequency, BSInnerTime, dataRange, &optimalCost,
                        &optimalStruct);
  }
  MapKey key = {true, {dataRange.initRange.left, dataRange.initRange.size}};
  if (optimalCost.time < DBL_MAX) structMap.insert({key, optimalStruct});
  nodeCost = {optimalCost.time, optimalCost.space, optimalCost.cost, true};
  return nodeCost;
}

#endif  // !DP_INNER_H