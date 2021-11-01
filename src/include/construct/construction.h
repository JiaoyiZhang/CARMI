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
#ifndef CONSTRUCT_CONSTRUCTION_H_
#define CONSTRUCT_CONSTRUCTION_H_

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
    const SubDataset &subDataset) {
  int nowBlockIdx = 0;
  // the number of data points which can be prefetched
  int prefetchNum = 0;
  // the index of all leaf nodes which can be prefetched
  std::vector<int> prefetchNode;
  // the range of all leaf nodes which can be prefetched
  std::vector<DataRange> prefetchRange;
  // used to train the prefetch prediction model, each element is <leaf node id,
  // block number>
  std::vector<std::pair<double, int>> prefetchData;
  // the cost of all leaf nodes in different allocated number of data blocks
  std::map<int, std::vector<double>> CostPerLeaf;

  for (int i = 0; i < static_cast<int>(subDataset.subInit.size()); i++) {
    COST.insert({emptyRange, emptyCost});

    NodeCost resChild;
    DataRange range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);

    // choose the suitable algorithm to construct the sub-tree according to the
    // size of the sub-dataset
    if (subDataset.subInit[i].size + subDataset.subInsert[i].size >
        carmi_params::kAlgorithmThreshold)
      resChild = GreedyAlgorithm(range);
    else
      resChild = DP(range);

    auto it = structMap.find(range.initRange);
    // if this node is the cf array leaf node, prepare for the prefetch function
    if ((it->second.cfArray.flagNumber >> 24) == ARRAY_LEAF_NODE &&
        range.initRange.left != -1) {
      int end = range.initRange.left + range.initRange.size;
      prefetchNum += range.initRange.size;
      int neededBlockNum = CFArrayType<KeyType, ValueType>::CalNeededBlockNum(
          range.initRange.size + range.insertRange.size);
      int avg =
          std::max(1.0, ceil((range.initRange.size + range.insertRange.size) *
                             1.0 / neededBlockNum));
      // construct the prefetch dataset: {the unrounded leaf node index, the
      // index of the data block}
      for (int j = range.initRange.left, k = 1; j < end; j++, k++) {
        double preIdx = root.model.Predict(initDataset[j].first);
        prefetchData.push_back({preIdx, nowBlockIdx});
        if (k == avg || j == end - 1) {
          k = 0;
          nowBlockIdx++;
        }
      }
      scanLeaf.push_back(i);
      lastLeaf = i;
      // add this node to the prefetchNode/Range
      prefetchNode.push_back(i);
      prefetchRange.push_back(range);
    } else {
      // store the other optimal sub-trees
      StoreOptimalNode(range, i);
    }

    std::map<IndexPair, NodeCost>().swap(COST);
    std::map<IndexPair, BaseNode<KeyType, ValueType>>().swap(structMap);
  }

  if (!isPrimary) {
    // calculate the cost of all prefetched leaf nodes in different number of
    // data blocks
    for (int i = 0; i < prefetchNode.size(); i++) {
      std::vector<double> cost = CalculateCFArrayCost(
          prefetchRange[i].initRange.size + prefetchRange[i].insertRange.size,
          prefetchNum);
      CostPerLeaf.insert({prefetchNode[i], cost});
    }
    int newBlockSize =
        root.fetch_model.PrefetchTrain(prefetchData, CostPerLeaf);
    data.dataArray.resize(newBlockSize, LeafSlots<KeyType, ValueType>());

    // store the leaf nodes which can be prefetched
    for (int i = 0; i < prefetchNode.size(); i++) {
      CFArrayType<KeyType, ValueType> currnode;
      int neededBlockNum = CFArrayType<KeyType, ValueType>::CalNeededBlockNum(
          prefetchRange[i].initRange.size + prefetchRange[i].insertRange.size);
      int predictBlockNum = root.fetch_model.PrefetchNum(prefetchNode[i]);
      bool isSuccess = false;
      if (neededBlockNum <= predictBlockNum) {
        std::vector<int> prefetchIndex(prefetchRange[i].initRange.size);
        int s = prefetchRange[i].initRange.left;
        int e =
            prefetchRange[i].initRange.left + prefetchRange[i].initRange.size;
        for (int j = s; j < e; j++) {
          double predictLeafIdx = root.model.Predict(initDataset[j].first);
          int p = root.fetch_model.PrefetchPredict(predictLeafIdx);
          prefetchIndex[j - s] = p;
        }
        isSuccess = currnode.StoreData(initDataset, prefetchIndex, true,
                                       predictBlockNum, s, &data, &prefetchEnd);
      }
      if (!isSuccess) {
        remainingNode.push_back(prefetchNode[i]);
        remainingRange.push_back(prefetchRange[i]);
      } else {
        node.nodeArray[prefetchNode[i]].cfArray = currnode;
      }
    }
  }

  isInitMode = false;
  // store the remaining node which cannot be prefetched
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
        double predictLeafIdx = root.model.Predict(initDataset[j].first);
        int p = root.fetch_model.PrefetchPredict(predictLeafIdx);
        prefetchIndex[j - s] = p;
      }

      currnode.StoreData(initDataset, prefetchIndex, isInitMode, neededBlockNum,
                         s, &data, &prefetchEnd);
      node.nodeArray[remainingNode[i]].cfArray = currnode;
    }
  }

  std::vector<int>().swap(remainingNode);
  std::vector<DataRange>().swap(remainingRange);
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Construction() {
  // choose the optimal setting of the root node
  // RootStruct res = ChooseRoot();
  RootStruct res = {0, 904349};
  // construct and store the root node, obtain the range of each sub-dataset
  SubDataset subDataset = StoreRoot(res);

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

  // construct each sub-tree
  ConstructSubTree(subDataset);

  // release useless memory
  int neededSize = data.usedDatasize + reservedSpace;
  if (!isPrimary) {
    UpdateLeaf();
    data.ReleaseUselessMemory(neededSize);
  }

  neededSize = node.nowNodeNumber + reservedSpace;
  if (neededSize < static_cast<int>(node.nodeArray.size())) {
    node.ReleaseUselessMemory(neededSize);
  }
  prefetchEnd = -1;
  DataVectorType().swap(initDataset);
  QueryType().swap(findQuery);
  KeyVectorType().swap(insertQuery);
}

#endif  // CONSTRUCT_CONSTRUCTION_H_
