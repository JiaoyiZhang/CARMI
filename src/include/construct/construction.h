/**
 * @file construction.h
 * @author Jiaoyi
 * @brief main functions for CARMI
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_CONSTRUCTION_H_
#define SRC_INCLUDE_CONSTRUCT_CONSTRUCTION_H_

#include <map>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "./construct_root.h"
#include "./structures.h"

/**
 * @brief construct each subtree using dp/greedy
 *
 * @param rootStruct the type and childNumber of root
 * @param subDataset the left and size of data points in each child node
 * @param nodeCost the space, time, cost of the index (is added ...)
 */
template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::ConstructSubTree(
    const RootStruct &rootStruct, const SubDataset &subDataset,
    NodeCost *nodeCost) {
  for (int i = 0; i < rootStruct.rootChildNum; i++) {
    COST.insert({emptyRange, emptyCost});

    NodeCost resChild;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size > kAlgorithmThreshold)
      resChild = GreedyAlgorithm(range);
    else
      resChild = DP(range);

    StoreOptimalNode(i, range);

    nodeCost->cost += resChild.space * kRate + resChild.time;
    nodeCost->time += resChild.time;
    nodeCost->space += resChild.space;

    COST.clear();
    structMap.clear();
  }
}

/**
 * @brief main function of construction
 * @param initData the dataset used to initialize the index
 * @param findData the find queries used to training CARMI
 * @param insertData the insert queries used to training CARMI
 */
template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Construction(
    const DataVectorType &initData, const DataVectorType &findData,
    const DataVectorType &insertData) {
  NodeCost nodeCost = emptyCost;
  RootStruct res = ChooseRoot();
  rootType = res.rootType;
  SubDataset subDataset = StoreRoot(res, &nodeCost);

  ConstructSubTree(res, subDataset, &nodeCost);
  UpdateLeaf();

  if (!carmi_params::kPrimaryIndex) {
    entireData.erase(
        entireData.begin() + nowDataSize + carmi_params::kReservedSpace,
        entireData.end());
  }
  DataVectorType().swap(initDataset);
  DataVectorType().swap(findQuery);
  DataVectorType().swap(insertQuery);
  std::vector<int>().swap(insertQueryIndex);

#ifdef DEBUG
  std::cout << "Construction over!" << std::endl;
  std::cout << "total cost: " << nodeCost.cost << std::endl;
  std::cout << "total time: " << nodeCost.time << std::endl;
  std::cout << "total space: " << nodeCost.space << std::endl;
#endif
}

#endif  // SRC_INCLUDE_CONSTRUCT_CONSTRUCTION_H_
