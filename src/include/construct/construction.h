/**
 * @file construction.h
 * @author Jiaoyi
 * @brief main functions for CARMI
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_CONSTRUCTION_H_
#define SRC_INCLUDE_CONSTRUCT_CONSTRUCTION_H_

#include <algorithm>
#include <iomanip>
#include <map>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "./construct_root.h"
#include "./structures.h"

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::ConstructSubTree(
    const SubDataset &subDataset, NodeCost *nodeCost) {
  float tmp = 0;
  int preI = 0;
  int checkpoint = -1;
  int dataNum = 0;
  int prefetchNum = 0;
  std::map<int, std::vector<double>> CostPerLeaf;
  std::vector<int> prefetchNode;
  std::vector<DataRange> prefetchRange;

  for (int i = 0; i < subDataset.subInit.size(); i++) {
    COST.insert({emptyRange, emptyCost});

    NodeCost resChild;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);

    if (subDataset.subInit[i].size + subDataset.subInsert[i].size >
        carmi_params::kAlgorithmThreshold)
      resChild = GreedyAlgorithm(range);
    else
      resChild = DP(range);

    // for prefetch
    auto it = structMap.find(range.initRange);
    if ((it->second.cfArray.flagNumber >> 24) == ARRAY_LEAF_NODE &&
        range.initRange.left != -1) {
      int end = range.initRange.left + range.initRange.size;
      prefetchNum += range.initRange.size;
      int neededBlockNum = CFArrayType<KeyType, ValueType>::CalNeededBlockNum(
          range.initRange.size + range.insertRange.size);
      if (i - preI > 10000) {
        checkpoint = prefetchData.size();
      }

      int avg =
          std::max(1.0, ceil((range.initRange.size + range.insertRange.size) *
                             1.0 / neededBlockNum));
      for (int j = range.initRange.left, k = 1; j < end; j++, k++) {
        double preIdx = root.model.PredictIdx(initDataset[j].first);
        prefetchData.push_back({preIdx, tmp});
        if (k == avg || j == end - 1) {
          k = 0;
          tmp++;
        }
        preI = i;
      }
      prefetchNode.push_back(i);
      prefetchRange.push_back(range);
      dataNum += range.initRange.size;
    } else {
      StoreOptimalNode(i, range);
    }

    nodeCost->cost += resChild.space * lambda + resChild.time;
    nodeCost->time += resChild.time;
    nodeCost->space += resChild.space;

    std::map<IndexPair, NodeCost>().swap(COST);
    std::map<IndexPair, BaseNode<KeyType, ValueType>>().swap(structMap);
  }

  for (int i = 0; i < prefetchNode.size(); i++) {
    // calculate the cost of this array
    std::vector<double> cost(CFArrayType<KeyType, ValueType>::kMaxBlockCapacity,
                             0);
    for (int k = 0; k < CFArrayType<KeyType, ValueType>::kMaxBlockCapacity;
         k++) {
      double tmp_cost = CalculateCFArrayCost(
          prefetchRange[i].initRange.size + prefetchRange[i].insertRange.size,
          prefetchNum, k + 1);
      cost[k] = tmp_cost;
    }
    CostPerLeaf.insert({prefetchNode[i], cost});
  }
  if (!isPrimary) {
    int newBlockSize =
        root.fetch_model.PrefetchTrain(prefetchData, CostPerLeaf, checkpoint);
    data.dataArray.resize(newBlockSize, LeafSlots<KeyType, ValueType>());
  }

  for (int i = 0; i < prefetchNode.size(); i++) {
    CFArrayType<KeyType, ValueType> currnode;
    int neededBlockNum = CFArrayType<KeyType, ValueType>::CalNeededBlockNum(
        prefetchRange[i].initRange.size + prefetchRange[i].insertRange.size);
    int predictBlockNum = root.fetch_model.PrefetchNum(prefetchNode[i]);
    bool isSuccess = false;
    if (neededBlockNum <= predictBlockNum) {
      std::vector<int> prefetchIndex(prefetchRange[i].initRange.size);
      int s = prefetchRange[i].initRange.left;
      int e = prefetchRange[i].initRange.left + prefetchRange[i].initRange.size;
      for (int j = s; j < e; j++) {
        double predictLeafIdx = root.model.PredictIdx(initDataset[j].first);
        int p = root.fetch_model.PrefetchPredict(predictLeafIdx);
        prefetchIndex[j - s] = p;
      }
      isSuccess =
          currnode.StoreData(initDataset, prefetchIndex, true, predictBlockNum,
                             s, e - s, &data, &prefetchEnd);
    }
    if (!isSuccess) {
      remainingNode.push_back(prefetchNode[i]);
      remainingRange.push_back(prefetchRange[i]);
    } else {
      node.nodeArray[prefetchNode[i]].cfArray = currnode;
      scanLeaf.push_back(prefetchNode[i]);
    }
  }
  isInitMode = false;
  if (remainingNode.size() > 0) {
    for (int i = 0; i < remainingNode.size(); i++) {
      CFArrayType<KeyType, ValueType> currnode;
      int neededBlockNum = CFArrayType<KeyType, ValueType>::CalNeededBlockNum(
          remainingRange[i].initRange.size +
          remainingRange[i].insertRange.size);
      std::vector<int> prefetchIndex(remainingRange[i].initRange.size);
      int s = remainingRange[i].initRange.left;
      int e =
          remainingRange[i].initRange.left + remainingRange[i].initRange.size;
      for (int j = s; j < e; j++) {
        double predictLeafIdx = root.model.PredictIdx(initDataset[j].first);
        int p = root.fetch_model.PrefetchPredict(predictLeafIdx);
        prefetchIndex[j - s] = p;
      }

      currnode.StoreData(initDataset, prefetchIndex, isInitMode, neededBlockNum,
                         s, e - s, &data, &prefetchEnd);
      node.nodeArray[remainingNode[i]].cfArray = currnode;
      scanLeaf.push_back(remainingNode[i]);
    }
  }

  std::vector<int>().swap(remainingNode);
  std::vector<DataRange>().swap(remainingRange);
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Construction() {
  NodeCost nodeCost = emptyCost;
  RootStruct res = ChooseRoot();
  // RootStruct res = {0, 904349};
  SubDataset subDataset = StoreRoot(res, &nodeCost);

#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "constructing root is over!" << std::endl;
  std::cout << "the number of children is: " << res.rootChildNum << std::endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << std::endl;
#endif

  ConstructSubTree(subDataset, &nodeCost);
  UpdateLeaf();

  int neededSize = data.usedDatasize + reservedSpace;
  if (!isPrimary) {
    data.ReleaseUselessMemory(neededSize);
  }

  neededSize = node.nowNodeNumber + reservedSpace;
  if (neededSize < static_cast<int>(node.nodeArray.size())) {
    node.ReleaseUselessMemory(neededSize);
  }
  prefetchEnd = -1;
  DataVectorType().swap(initDataset);
  DataVectorType().swap(findQuery);
  DataVectorType().swap(insertQuery);
}

#endif  // SRC_INCLUDE_CONSTRUCT_CONSTRUCTION_H_
