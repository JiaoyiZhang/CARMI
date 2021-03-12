/**
 * @file construction.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_CONSTRUCT_CONSTRUCTION_H_
#define SRC_CARMI_CONSTRUCT_CONSTRUCTION_H_

#include <map>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "./construct_root.h"
#include "./structures.h"

/**
 * @brief construct each subtree using dp/greedy
 * @param rootStruct the type and childNumber of root
 * @param subDataset the left and size of data points in each child node
 * @param nodeCost the space, time, cost of the index (is added ...)
 */
inline void CARMI::ConstructSubTree(const RootStruct &rootStruct,
                                    const SubDataset &subDataset,
                                    NodeCost *nodeCost) {
  for (int i = 0; i < rootStruct.rootChildNum; i++) {
    COST.insert({emptyRange, emptyCost});
    structMap.insert({(MapKey){false, emptyRange}, leafP});

    NodeCost resChild;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size > kAlgorithmThreshold)
      resChild = GreedyAlgorithm(range);
    else
      resChild = dp(range);

    MapKey key = {resChild.isInnerNode,
                  {subDataset.subInit[i].size, subDataset.subInsert[i].size}};
    auto it = structMap.find(key);
    int type = it->second.type;

    storeOptimalNode(i, type, key, range);

    nodeCost->cost += resChild.space + resChild.time;
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
 * @return the type of root
 */
inline void CARMI::Construction(const DataVectorType &initData,
                                const DataVectorType &findData,
                                const DataVectorType &insertData) {
  NodeCost nodeCost = {0, 0, 0, true};
  RootStruct res = ChooseRoot();
  // RootStruct res = RootStruct(0, 131072);
  rootType = res.rootType;
  SubDataset subDataset = StoreRoot(res, &nodeCost);

  ConstructSubTree(res, subDataset, &nodeCost);
  UpdateLeaf();

  if (kPrimaryIndex) {
    DataVectorType tmp(100000, {DBL_MIN, DBL_MIN});
    externalData.insert(externalData.end(), tmp.begin(), tmp.end());
    nowDataSize += 100000;
    DataVectorType().swap(entireData);
  } else {
    entireData.erase(entireData.begin() + nowDataSize + reservedSpace,
                     entireData.end());
  }
  DataVectorType().swap(initDataset);
  DataVectorType().swap(findQuery);
  DataVectorType().swap(insertQuery);

#ifdef DEBUG
  std::cout << "Construction over!" << std::endl;
  std::cout << "total cost: " << nodeCost.cost << std::endl;
  std::cout << "total time: " << nodeCost.time << std::endl;
  std::cout << "total space: " << nodeCost.space << std::endl;
#endif
}

#endif  // SRC_CARMI_CONSTRUCT_CONSTRUCTION_H_
