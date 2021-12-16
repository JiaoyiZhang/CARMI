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

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline void CARMI<KeyType, ValueType, Compare, Alloc>::ConstructSubTree(
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
      prefetchNum += range.initRange.size;
      int totalSize = range.initRange.size + range.insertRange.size;
      int neededBlockNum =
          CFArrayType<KeyType, ValueType, Compare, Alloc>::CalNeededBlockNum(
              totalSize);
      int avg = std::max(1.0, ceil((totalSize)*1.0 / neededBlockNum));

      // use the init data points and the future inserted data points to train
      // the leaf node, but we only store the init data points into the leaf
      // node
      QueryType initInsertData(totalSize);
      int cnt = 0;
      int end = range.initRange.left + range.initRange.size;
      for (int j = range.initRange.left; j < end; j++) {
        initInsertData[cnt++] = {initDataset[j].first, 1};
      }
      end = range.insertRange.left + range.insertRange.size;
      for (int j = range.insertRange.left; j < end; j++) {
        initInsertData[cnt++] = {insertQuery[j], 0};
      }
      std::sort(initInsertData.begin(), initInsertData.end());

      // construct the prefetch dataset: {the unrounded leaf node index, the
      // index of the data block}
      for (int j = 0, k = 1; j < totalSize; j++, k++) {
        if (initInsertData[j].second == 1) {
          double preIdx = root.model.Predict(initInsertData[j].first);
          prefetchData.push_back({preIdx, nowBlockIdx});
        }
        if (k == avg || j == totalSize - 1) {
          k = 0;
          nowBlockIdx++;
        }
      }
      scanLeaf.push_back(i);

      // add this node to the prefetchNode/Range
      prefetchNode.push_back(i);
      prefetchRange.push_back(range);
      if (firstLeaf == -1 && range.initRange.size > 0) {
        firstLeaf = i;
      }
      if (range.initRange.size > 0) {
        lastLeaf = i;
      }
    } else {
      // store the other optimal sub-trees
      StoreOptimalNode(range, i);
    }

    std::map<IndexPair, NodeCost>().swap(COST);
    std::map<IndexPair, BaseNode<KeyType, ValueType, Compare, Alloc>>().swap(
        structMap);
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
      CFArrayType<KeyType, ValueType, Compare, Alloc> currnode;
      int totalSize =
          prefetchRange[i].initRange.size + prefetchRange[i].insertRange.size;
      int neededBlockNum =
          CFArrayType<KeyType, ValueType, Compare, Alloc>::CalNeededBlockNum(
              totalSize);
      int predictBlockNum = root.fetch_model.PrefetchNum(prefetchNode[i]);
      bool isSuccess = false;
      if (neededBlockNum <= predictBlockNum) {
        std::vector<int> prefetchIndex(totalSize);
        int s = prefetchRange[i].initRange.left;
        int e = s + prefetchRange[i].initRange.size;
        DataVectorType tmpDataset(initDataset.begin() + s,
                                  initDataset.begin() + e);
        s = prefetchRange[i].insertRange.left;
        e = s + prefetchRange[i].insertRange.size;
        if (prefetchRange[i].insertRange.size > 0) {
          for (int j = s; j < e; j++) {
            tmpDataset.push_back(
                {insertQuery[j], static_cast<ValueType>(DBL_MAX)});
          }
          std::sort(tmpDataset.begin(), tmpDataset.end());
        }
        for (int j = 0; j < totalSize; j++) {
          double predictLeafIdx = root.model.Predict(tmpDataset[j].first);
          int p = root.fetch_model.PrefetchPredict(predictLeafIdx);
          prefetchIndex[j] = p;
        }
        isSuccess = currnode.StoreData(
            tmpDataset, prefetchIndex, true, predictBlockNum, 0,
            prefetchRange[i].initRange.size, &data, &prefetchEnd);
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
  for (int i = 0; i < remainingNode.size(); i++) {
    CFArrayType<KeyType, ValueType, Compare, Alloc> currnode;
    int totalSize =
        remainingRange[i].initRange.size + remainingRange[i].insertRange.size;
    int neededBlockNum =
        CFArrayType<KeyType, ValueType, Compare, Alloc>::CalNeededBlockNum(
            totalSize);
    std::vector<int> prefetchIndex(totalSize);
    int s = remainingRange[i].initRange.left;
    int e = s + remainingRange[i].initRange.size;
    DataVectorType tmpDataset(initDataset.begin() + s, initDataset.begin() + e);
    if (remainingRange[i].insertRange.size > 0) {
      s = remainingRange[i].insertRange.left;
      e = s + remainingRange[i].insertRange.size;
      for (int j = s; j < e; j++) {
        tmpDataset.push_back({insertQuery[j], static_cast<ValueType>(DBL_MAX)});
      }
      std::sort(tmpDataset.begin(), tmpDataset.end());
    }
    for (int j = 0; j < totalSize; j++) {
      double predictLeafIdx = root.model.Predict(tmpDataset[j].first);
      int p = root.fetch_model.PrefetchPredict(predictLeafIdx);
      prefetchIndex[j] = p;
    }
    currnode.StoreData(tmpDataset, prefetchIndex, isInitMode, neededBlockNum, 0,
                       remainingRange[i].initRange.size, &data, &prefetchEnd);

    node.nodeArray[remainingNode[i]].cfArray = currnode;
  }

  std::vector<int>().swap(remainingNode);
  std::vector<DataRange>().swap(remainingRange);
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline void CARMI<KeyType, ValueType, Compare, Alloc>::Construction() {
  // choose the optimal setting of the root node
  RootStruct res = ChooseRoot();
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
