/**
 * @file construct_root.h
 * @author Jiaoyi
 * @brief functions for constructing the root
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CONSTRUCT_CONSTRUCT_ROOT_H_
#define CONSTRUCT_CONSTRUCT_ROOT_H_
#include <algorithm>
#include <vector>

#include "../carmi.h"
#include "../nodes/rootNode/trainModel/linear_regression.h"
#include "./dp.h"
#include "./store_node.h"
#include "./structures.h"

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
template <typename RootNodeType>
void CARMI<KeyType, ValueType, Compare, Alloc>::UpdateRootOptSetting(
    int c, double *optimalCost, RootStruct *rootStruct) {
  // calculate the basic space cost of the c child nodes of the root node
  double space_cost = kBaseNodeSpace * static_cast<double>(c);
  // calculate the time cost of the root node
  double time_cost = RootNodeType::kTimeCost;

  // train this type of the root node
  RootNodeType tmpRoot(c, initDataset);
  IndexPair range{0, static_cast<int>(initDataset.size())};
  IndexPair insertRange{0, static_cast<int>(insertQuery.size())};
  // initialize the variables that store the range of each sub-dataset
  std::vector<IndexPair> perSize(c, emptyRange);
  std::vector<IndexPair> perInsertSize(c, emptyRange);
  // split initDataset into c sub-datasets
  NodePartition<typename RootNodeType::ModelType>(tmpRoot.model, range,
                                                  initDataset, &perSize);
  // split insertDataset into c sub-datasets
  NodePartition<typename RootNodeType::ModelType>(tmpRoot.model, insertRange,
                                                  insertQuery, &perInsertSize);

  int maxLeafCapacity = carmi_params::kMaxLeafNodeSizeExternal;
  if (!isPrimary) {
    maxLeafCapacity =
        CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxLeafCapacity;
  }
  for (int i = 0; i < c; i++) {
    if (perSize[i].size == static_cast<int>(initDataset.size())) {
      return;
    }
    int totalDataNum = perSize[i].size + perInsertSize[i].size;
    // if leaf nodes are cf array leaf nodes, add the space cost of data
    // blocks to the total space cost
    if (!isPrimary) {
      int tmpBlockNum =
          CFArrayType<KeyType, ValueType, Compare, Alloc>::CalNeededBlockNum(
              totalDataNum);
      space_cost +=
          tmpBlockNum * carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
    }
    // if the total number of data points exceeds the maximum capacity of the
    // leaf node, the current node needs at least kMinChildNumber inner nodes to
    // manage the data points together
    if (totalDataNum > maxLeafCapacity) {
      space_cost += kBaseNodeSpace * kMinChildNumber;
      time_cost += carmi_params::kMemoryAccessTime *
                   static_cast<double>(perSize[i].size) /
                   static_cast<double>(initDataset.size());
    }
  }

  // calculate the entropy of the root node
  double entropy = CalculateEntropy(perSize);
  double cost =
      (time_cost + lambda * static_cast<double>(space_cost)) / entropy;

  // if the current cost is smaller than the optimal cost, update the optimal
  // cost and root setting
  if (cost <= *optimalCost) {
    *optimalCost = cost;
    rootStruct->rootChildNum = c;
    rootStruct->rootType = tmpRoot.flagNumber;
  }
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
RootStruct CARMI<KeyType, ValueType, Compare, Alloc>::ChooseRoot() {
  double OptimalValue = DBL_MAX;
  RootStruct rootStruct(PLR_ROOT_NODE, kMinChildNumber);
  int minNum =
      std::max(kMinChildNumber, static_cast<int>(initDataset.size() / 1024));
  int maxNum =
      std::max(kMinChildNumber, static_cast<int>(initDataset.size() / 2));

  // Calculate the cost of different settings and choose the optimal setting
  for (int c = minNum; c <= maxNum; c *= 1.3) {
    UpdateRootOptSetting<PLRType<DataVectorType, KeyType>>(
        c * 1.001, &OptimalValue, &rootStruct);
  }
  // return the optimal root setting
  return rootStruct;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
SubDataset CARMI<KeyType, ValueType, Compare, Alloc>::StoreRoot(
    const RootStruct &rootStruct) {
  SubDataset subDataset(rootStruct.rootChildNum);
  // allocate a block of empty memory for these child nodes
  node.AllocateNodeMemory(rootStruct.rootChildNum);
  DataRange range({0, static_cast<int>(initDataset.size())},
                  {0, static_cast<int>(findQuery.size())},
                  {0, static_cast<int>(insertQuery.size())});
  switch (rootStruct.rootType) {
    case PLR_ROOT_NODE: {
      // construct the root node and train the model
      root = PLRType<DataVectorType, KeyType>(rootStruct.rootChildNum,
                                              initDataset);
      // split the dataset
      NodePartition<typename PLRType<DataVectorType, KeyType>::ModelType>(
          root.model, range.initRange, initDataset, &(subDataset.subInit));
      subDataset.subFind = subDataset.subInit;
      NodePartition<typename PLRType<DataVectorType, KeyType>::ModelType>(
          root.model, range.insertRange, insertQuery, &(subDataset.subInsert));
      break;
    }
  }
  // roughly calculate the number of needed data blocks
  int blockNum = 0;
  for (int i = 0; i < rootStruct.rootChildNum; i++) {
    if (subDataset.subInit[i].size + subDataset.subInsert[i].size <
        CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxLeafCapacity)
      blockNum +=
          CFArrayType<KeyType, ValueType, Compare, Alloc>::CalNeededBlockNum(
              subDataset.subInit[i].size + subDataset.subInsert[i].size);
  }

  // update the block number of the prefetch prediction model
  root.fetch_model.SetBlockNumber(blockNum);
  // update the size of the data array
  data.dataArray.resize(blockNum, LeafSlots<KeyType, ValueType>());
  return subDataset;
}
#endif  // CONSTRUCT_CONSTRUCT_ROOT_H_
