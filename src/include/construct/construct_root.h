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

template <typename KeyType, typename ValueType>
template <typename RootNodeType>
void CARMI<KeyType, ValueType>::IsBetterRoot(int c, NodeType type,
                                             double *optimalCost,
                                             RootStruct *rootStruct) {
  std::vector<IndexPair> perSize(c, emptyRange);
  std::vector<IndexPair> perInsertSize(c, emptyRange);
  double space_cost = kBaseNodeSpace * c;
  double time_cost = RootNodeType::kTimeCost;

  RootNodeType tmpRoot(c);
  tmpRoot.model.Train(initDataset);
  IndexPair range{0, static_cast<int>(initDataset.size())};
  IndexPair insertRange{0, static_cast<int>(insertQuery.size())};
  NodePartition<typename RootNodeType::ModelType>(tmpRoot.model, range,
                                                  initDataset, &perSize);
  NodePartition<typename RootNodeType::ModelType>(tmpRoot.model, insertRange,
                                                  insertQuery, &perInsertSize);

  for (int i = 0; i < c; i++) {
    int maxLeafCapacity = carmi_params::kMaxLeafNodeSizeExternal;
    int totalDataNum = perSize[i].size + perInsertSize[i].size;
    if (!isPrimary) {
      int tmpBlockNum =
          CFArrayType<KeyType, ValueType>::CalNeededBlockNum(totalDataNum);
      space_cost +=
          tmpBlockNum * carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
      maxLeafCapacity = CFArrayType<KeyType, ValueType>::kMaxLeafCapacity;
    }
    if (totalDataNum > maxLeafCapacity) {
      space_cost += kBaseNodeSpace * kMinChildNumber;
      time_cost += carmi_params::kMemoryAccessTime * perSize[i].size /
                   initDataset.size();
    }
  }

  double entropy = CalculateEntropy(initDataset.size(), c, perSize);
  double cost = (time_cost + static_cast<float>(lambda * space_cost)) / entropy;

  if (cost <= *optimalCost) {
    *optimalCost = cost;
    rootStruct->rootChildNum = c;
    rootStruct->rootType = type;
  }
}

template <typename KeyType, typename ValueType>
RootStruct CARMI<KeyType, ValueType>::ChooseRoot() {
  double OptimalValue = DBL_MAX;
  RootStruct rootStruct(0, 0);
  int minNum =
      std::max(kMinChildNumber, static_cast<int>(initDataset.size() / 1024));
  int maxNum =
      std::max(kMinChildNumber, static_cast<int>(initDataset.size() / 2));
  for (int c = minNum; c <= maxNum; c *= 1.3) {
    IsBetterRoot<PLRType<DataVectorType, KeyType>>(c * 1.001, PLR_ROOT_NODE,
                                                   &OptimalValue, &rootStruct);
  }
  return rootStruct;
}

template <typename KeyType, typename ValueType>
SubDataset CARMI<KeyType, ValueType>::StoreRoot(const RootStruct &rootStruct,
                                                NodeCost *nodeCost) {
  SubDataset subDataset(rootStruct.rootChildNum);
  node.AllocateNodeMemory(rootStruct.rootChildNum);
  DataRange range({0, static_cast<int>(initDataset.size())},
                  {0, static_cast<int>(findQuery.size())},
                  {0, static_cast<int>(insertQuery.size())});
  switch (rootStruct.rootType) {
    case PLR_ROOT_NODE: {
      nodeCost->time = carmi_params::kPLRRootTime;
      nodeCost->space += sizeof(PLRType<DataVectorType, KeyType>);
      root = PLRType<DataVectorType, KeyType>(rootStruct.rootChildNum,
                                              initDataset);
      NodePartition<typename PLRType<DataVectorType, KeyType>::ModelType>(
          root.model, range.initRange, initDataset, &(subDataset.subInit));
      subDataset.subFind = subDataset.subInit;
      NodePartition<typename PLRType<DataVectorType, KeyType>::ModelType>(
          root.model, range.insertRange, insertQuery, &(subDataset.subInsert));
      break;
    }
  }
  int blockNum = 0;
  for (int i = 0; i < rootStruct.rootChildNum; i++) {
    if (subDataset.subInit[i].size + subDataset.subInsert[i].size <
        CFArrayType<KeyType, ValueType>::kMaxLeafCapacity)
      blockNum += CFArrayType<KeyType, ValueType>::CalNeededBlockNum(
          subDataset.subInit[i].size + subDataset.subInsert[i].size);
  }

  root.fetch_model.SetBlockNumber(blockNum);
  data.dataArray.resize(blockNum, LeafSlots<KeyType, ValueType>());
  return subDataset;
}
#endif  // CONSTRUCT_CONSTRUCT_ROOT_H_
