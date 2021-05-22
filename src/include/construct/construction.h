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

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::ConstructSubTree(
    const RootStruct &rootStruct, const SubDataset &subDataset,
    NodeCost *nodeCost) {
  for (int i = 0; i < rootStruct.rootChildNum; i++) {
    COST.insert({emptyRange, emptyCost});

    NodeCost resChild;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size > carmi_params::kAlgorithmThreshold)
      resChild = GreedyAlgorithm(range);
    else
      resChild = DP(range);

    StoreOptimalNode(i, range);

    nodeCost->cost += resChild.space * lambda + resChild.time;
    nodeCost->time += resChild.time;
    nodeCost->space += resChild.space;

    // COST.clear();
    std::map<IndexPair, NodeCost>().swap(COST);
    std::map<IndexPair, BaseNode>().swap(structMap);
    // structMap.clear();
  }
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Construction(
    const DataVectorType &initData, const DataVectorType &findData,
    const DataVectorType &insertData) {
  NodeCost nodeCost = emptyCost;
  RootStruct res = ChooseRoot();
  rootType = res.rootType;
  SubDataset subDataset = StoreRoot(res, &nodeCost);
  
#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "constructing root is over!" << std::endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << std::endl;
#endif

  ConstructSubTree(res, subDataset, &nodeCost);
  UpdateLeaf();

  int neededSize = nowDataSize + reservedSpace;
  if (!isPrimary) {
    if (neededSize < static_cast<int>(entireData.size())) {
      DataVectorType tmpEntireData(entireData.begin(),
                                   entireData.begin() + neededSize);
      DataVectorType().swap(entireData);
      entireData = tmpEntireData;
    }

    for (int i = 0; i < static_cast<int>(emptyBlocks.size()); i++) {
      auto it = emptyBlocks[i].m_block.lower_bound(neededSize);
      emptyBlocks[i].m_block.erase(it, emptyBlocks[i].m_block.end());
    }
  }

  neededSize = nowChildNumber + reservedSpace;
  if (neededSize < static_cast<int>(entireChild.size())) {
    std::vector<BaseNode> tmp(entireChild.begin(),
                              entireChild.begin() + neededSize);
    std::vector<BaseNode>().swap(entireChild);
    entireChild = tmp;
  }

  DataVectorType().swap(initDataset);
  DataVectorType().swap(findQuery);
  DataVectorType().swap(insertQuery);
  std::vector<int>().swap(insertQueryIndex);
}

#endif  // SRC_INCLUDE_CONSTRUCT_CONSTRUCTION_H_