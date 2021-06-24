/**
 * @file construct_root.h
 * @author Jiaoyi
 * @brief functions for constructing the root
 * @version 0.1
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
  root.model.Train(initDataset, c);
  IndexPair range(0, initDataset.size());
  NodePartition<ModelType>(root.model, range, initDataset, &perSize);
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
  for (int c = minNum; c <= maxNum; c *= 2) {
    IsBetterRoot<LRType<DataVectorType, KeyType>,
                 LinearRegression<DataVectorType, KeyType> >(
        c, LR_ROOT_NODE, carmi_params::kLRRootTime, &OptimalValue, &rootStruct);
  }
  return rootStruct;
}

template <typename KeyType, typename ValueType>
template <typename TYPE, typename ModelType>
TYPE CARMI<KeyType, ValueType>::ConstructRoot(const RootStruct &rootStruct,
                                              const DataRange &range,
                                              SubDataset *subDataset) {
  TYPE root(rootStruct.rootChildNum);
  root.childLeft = AllocateChildMemory(rootStruct.rootChildNum);
  root.model.Train(initDataset, rootStruct.rootChildNum);

  NodePartition<ModelType>(root.model, range.initRange, initDataset,
                           &(subDataset->subInit));
  subDataset->subFind = subDataset->subInit;
  NodePartition<ModelType>(root.model, range.insertRange, insertQuery,
                           &(subDataset->subInsert));
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
    case LR_ROOT_NODE: {
      nodeCost->time = carmi_params::kLRRootTime;
      nodeCost->space += sizeof(LRType<DataVectorType, KeyType>);
      root = ConstructRoot<LRType<DataVectorType, KeyType>,
                           LinearRegression<DataVectorType, KeyType> >(
          rootStruct, dataRange, &subDataset);
      break;
    }
  }
  return subDataset;
}
#endif  // SRC_INCLUDE_CONSTRUCT_CONSTRUCT_ROOT_H_
