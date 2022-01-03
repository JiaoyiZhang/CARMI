/**
 * @file greedy.h
 * @author Jiaoyi
 * @brief use the greedy node selection algorithm to construct inner nodes
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CONSTRUCT_GREEDY_H_
#define CONSTRUCT_GREEDY_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../carmi.h"
#include "../params.h"
#include "./dp_inner.h"
#include "./minor_function.h"
#include "./structures.h"

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
template <typename InnerNodeType>
void CARMI<KeyType, ValueType, Compare, Alloc>::UpdateGreedyOptSetting(
    const DataRange &range, int c, double frequency_weight,
    NodeCost *optimalCost, InnerNodeType *optimal_node_struct) {
  // calculate the basic space cost of the c child nodes of the inner node
  double space_cost = kBaseNodeSpace * static_cast<double>(c);
  // calculate the time cost of the inner node
  double time_cost = InnerNodeType::kTimeCost;

  SubDataset subDataset(c);
  InnerNodeType currnode = InnerDivideAll<InnerNodeType>(range, c, &subDataset);
  int maxLeafCapacity = carmi_params::kMaxLeafNodeSizeExternal;
  if (!isPrimary) {
    maxLeafCapacity =
        CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxLeafCapacity;
  }
  for (int i = 0; i < c; i++) {
    int totalDataNum =
        subDataset.subInit[i].size + subDataset.subInsert[i].size;
    if (totalDataNum == range.initRange.size + range.insertRange.size) {
      return;
    }
    // if leaf nodes are cf array leaf nodes, add the space cost of data
    // blocks to the total space cost
    if (!isPrimary) {
      int tmpBlockNum =
          CFArrayType<KeyType, ValueType, Compare, Alloc>::CalNeededBlockNum(
              totalDataNum);
      space_cost += static_cast<double>(tmpBlockNum) *
                    carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
    }
    // if the total number of data points exceeds the maximum capacity of the
    // leaf node, the current node needs at least kMinChildNumber inner nodes to
    // manage the data points together
    if (totalDataNum > maxLeafCapacity) {
      space_cost += kBaseNodeSpace * kMinChildNumber;
      time_cost += carmi_params::kMemoryAccessTime *
                   static_cast<double>(subDataset.subInit[i].size) /
                   static_cast<double>(range.initRange.size);
    }
  }
  // calculate the entropy of the inner node
  double entropy = CalculateEntropy(subDataset.subInit);
  double cost = (time_cost + lambda * space_cost / frequency_weight) / entropy;

  // if the current cost is smaller than the optimal cost, update the optimal
  // cost and node setting
  if (cost <= optimalCost->cost) {
    *optimal_node_struct = currnode;
    *optimalCost = {time_cost, space_cost, cost};
  }
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
NodeCost CARMI<KeyType, ValueType, Compare, Alloc>::GreedyAlgorithm(
    const DataRange &dataRange) {
  // the optimal cost of this sub-dataset
  NodeCost optimalCost{DBL_MAX, DBL_MAX, DBL_MAX};
  // the optimal node of this sub-dataset
  BaseNode<KeyType, ValueType, Compare, Alloc> opt_struct;
  // calculate the weight of the frequency of this sub-dataset (findQuery and
  // insertQury)
  double frequency_weight = CalculateFrequencyWeight(dataRange);
  int tmpEnd = std::min(0x00FFFFFF, dataRange.initRange.size / 16);
  tmpEnd = std::max(tmpEnd, kMinChildNumber);
  for (int c = kMinChildNumber; c <= tmpEnd; c *= 2) {
    // Case 1: construct a LR inner node, if it is better than the current
    // optimal setting, then use it to update the optimal setting
    UpdateGreedyOptSetting<LRModel<KeyType, ValueType>>(
        dataRange, c, frequency_weight, &optimalCost, &(opt_struct.lr));
    // Case 2: construct a P. LR inner node, if it is better than the current
    // optimal setting, then use it to update the optimal setting
    UpdateGreedyOptSetting<PLRModel<KeyType, ValueType>>(
        dataRange, c, frequency_weight, &optimalCost, &(opt_struct.plr));
    // Case 3: construct a His inner node, if it is better than the current
    // optimal setting, then use it to update the optimal setting
    if (c <= kHisMaxChildNumber)
      UpdateGreedyOptSetting<HisModel<KeyType, ValueType>>(
          dataRange, c, frequency_weight, &optimalCost, &(opt_struct.his));
    // Case 4: construct a BS inner node, if it is better than the current
    // optimal setting, then use it to update the optimal setting
    if (c <= kBSMaxChildNumber)
      UpdateGreedyOptSetting<BSModel<KeyType, ValueType>>(
          dataRange, c, frequency_weight, &optimalCost, &(opt_struct.bs));
  }

  // use the optimal inner node to divide dataset into childNum sub-datasets
  int childNum = opt_struct.lr.flagNumber & 0x00FFFFFF;
  int type = opt_struct.lr.flagNumber >> 24;
  SubDataset subDataset(childNum);
  switch (type) {
    case LR_INNER_NODE: {
      InnerDivideAll<LRModel<KeyType, ValueType>>(dataRange, childNum,
                                                  &subDataset);
      break;
    }
    case PLR_INNER_NODE: {
      InnerDivideAll<PLRModel<KeyType, ValueType>>(dataRange, childNum,
                                                   &subDataset);
      break;
    }
    case HIS_INNER_NODE: {
      InnerDivideAll<HisModel<KeyType, ValueType>>(dataRange, childNum,
                                                   &subDataset);
      break;
    }
    case BS_INNER_NODE: {
      InnerDivideAll<BSModel<KeyType, ValueType>>(dataRange, childNum,
                                                  &subDataset);
      break;
    }
  }

  // recursively calculate the cost of the child nodes
  for (int i = 0; i < childNum; i++) {
    NodeCost res = emptyCost;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    // choose the suitable algorithm to construct the sub-tree according to the
    // size of the sub-dataset
    double minRatio = 0.95;
    // record the maximum capacity of the leaf node
    int maxStoredNum =
        CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxLeafCapacity;
    if (isPrimary) {
      maxStoredNum = carmi_params::kMaxLeafNodeSizeExternal;
    }
    if (range.initRange.size + range.insertRange.size <=
        minRatio * maxStoredNum) {
      // Case 3: if the size is smaller than the threshold, directly construct a
      // leaf node
      res = DPLeaf(range);
    } else if (subDataset.subInit[i].size + subDataset.subInsert[i].size >
               carmi_params::kAlgorithmThreshold) {
      res = GreedyAlgorithm(range);
    } else {
      res = DP(range);
    }
    optimalCost.cost += res.cost;
    optimalCost.time += res.time;
    optimalCost.space += res.space;
  }

  // store the optimal setting of this sub-dataset
  structMap.insert({dataRange.initRange, opt_struct});
  // store the minimum cost of this sub-dataset
  COST.insert({dataRange.initRange, optimalCost});
  return optimalCost;
}
#endif  // CONSTRUCT_GREEDY_H_
