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
#ifndef SRC_INCLUDE_CONSTRUCT_CONSTRUCT_ROOT_H_
#define SRC_INCLUDE_CONSTRUCT_CONSTRUCT_ROOT_H_
#include <algorithm>
#include <vector>

#include "../carmi.h"
#include "../nodes/rootNode/trainModel/linear_regression.h"
#include "./dp.h"
#include "./store_node.h"
#include "./structures.h"

template <typename KeyType, typename ValueType>
template <typename TYPE, typename ModelType>
void CARMI<KeyType, ValueType>::IsBetterRoot(int c, NodeType type,
                                             double time_cost,
                                             double *optimalCost,
                                             RootStruct *rootStruct) {
  std::vector<IndexPair> perSize(c, emptyRange);
  double space_cost = kBaseNodeSpace * c;

  TYPE root(c);
  root.model.Train(initDataset);
  IndexPair range{0, static_cast<int>(initDataset.size())};
  NodePartition<ModelType>(root.model, range, initDataset, &perSize);
  for (int i = 0; i < c; i++) {
    int maxLeafCapacity = carmi_params::kMaxLeafNodeSizeExternal;
    if (!isPrimary) {
      space_cost += CFArrayType<KeyType, ValueType>::CalculateNeededBlockNumber(
                        perSize[i].size) *
                    carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
      maxLeafCapacity = CFArrayType<KeyType, ValueType>::kMaxLeafCapacity;
    }
    if (perSize[i].size > maxLeafCapacity) {
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
    IsBetterRoot<PLRType<DataVectorType, KeyType>,
                 PiecewiseLR<DataVectorType, KeyType>>(
        c * 1.001, PLR_ROOT_NODE, carmi_params::kPLRRootTime, &OptimalValue,
        &rootStruct);
  }
  return rootStruct;
}

template <typename KeyType, typename ValueType>
template <typename TYPE, typename ModelType>
TYPE CARMI<KeyType, ValueType>::ConstructRoot(const RootStruct &rootStruct,
                                              const DataRange &range,
                                              SubDataset *subDataset) {
  TYPE root(rootStruct.rootChildNum);
  root.childLeft = node.AllocateNodeMemory(rootStruct.rootChildNum);
  root.model.Train(initDataset);

  NodePartition<ModelType>(root.model, range.initRange, initDataset,
                           &(subDataset->subInit));
  subDataset->subFind = subDataset->subInit;
  NodePartition<ModelType>(root.model, range.insertRange, insertQuery,
                           &(subDataset->subInsert));
  int blockNum = 0;
  for (int i = 0; i < rootStruct.rootChildNum; i++) {
    if (subDataset->subInit[i].size <
        CFArrayType<KeyType, ValueType>::kMaxLeafCapacity)
      blockNum += CFArrayType<KeyType, ValueType>::CalculateNeededBlockNumber(
          subDataset->subInit[i].size);
  }

  root.fetch_model.SetBlockNumber(blockNum);

  data.Resize(blockNum);
  return root;
}

template <typename KeyType, typename ValueType>
SubDataset CARMI<KeyType, ValueType>::StoreRoot(const RootStruct &rootStruct,
                                                NodeCost *nodeCost) {
  DataRange dataRange({0, static_cast<int>(initDataset.size())},
                      {0, static_cast<int>(findQuery.size())},
                      {0, static_cast<int>(insertQuery.size())});
  SubDataset subDataset(rootStruct.rootChildNum);
  switch (rootStruct.rootType) {
    case PLR_ROOT_NODE: {
      nodeCost->time = carmi_params::kPLRRootTime;
      nodeCost->space += sizeof(PLRType<DataVectorType, KeyType>);
      root = ConstructRoot<PLRType<DataVectorType, KeyType>,
                           PiecewiseLR<DataVectorType, KeyType>>(
          rootStruct, dataRange, &subDataset);
      break;
    }
  }
  return subDataset;
}
#endif  // SRC_INCLUDE_CONSTRUCT_CONSTRUCT_ROOT_H_
