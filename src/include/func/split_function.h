/**
 * @file split_function.h
 * @author Jiaoyi
 * @brief the split function for insert function
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef FUNC_SPLIT_FUNCTION_H_
#define FUNC_SPLIT_FUNCTION_H_

#include <float.h>

#include <algorithm>
#include <vector>

#include "../carmi.h"
#include "../construct/minor_function.h"
#include "../params.h"

template <typename KeyType, typename ValueType>
template <typename LeafNodeType>
inline void CARMI<KeyType, ValueType>::Split(int idx) {
  // get the parameters of this leaf node
  bool isExternal = node.nodeArray[idx].lr.flagNumber >> 24;
  int previousIdx = node.nodeArray[idx].cfArray.previousLeaf;
  int nextIdx = node.nodeArray[idx].cfArray.nextLeaf;

  DataVectorType tmpDataset;
  int leftIdx;
  // extract pure data points
  if (isExternal) {
    leftIdx = node.nodeArray[idx].externalArray.m_left;
    int rightIdx =
        leftIdx + (node.nodeArray[idx].externalArray.flagNumber & 0x00FFFFFF);
    tmpDataset = ExternalArray<KeyType>::ExtractDataset(external_data, leftIdx,
                                                        rightIdx, recordLength);
  } else {
    leftIdx = node.nodeArray[idx].cfArray.m_left;
    int rightIdx =
        leftIdx + (node.nodeArray[idx].cfArray.flagNumber & 0x00FFFFFF);
    tmpDataset = CFArrayType<KeyType, ValueType>::ExtractDataset(data, leftIdx,
                                                                 rightIdx);
  }
  int actualSize = tmpDataset.size();

  // create a new inner node and store it in the node[idx]
  auto currnode = LRModel<KeyType, ValueType>(kInsertNewChildNumber);
  currnode.childLeft = node.AllocateNodeMemory(kInsertNewChildNumber);
  currnode.Train(0, actualSize, tmpDataset);
  node.nodeArray[idx].lr = currnode;

  std::vector<IndexPair> perSize(kInsertNewChildNumber, emptyRange);
  IndexPair range{0, actualSize};
  NodePartition<LRModel<KeyType, ValueType>>(currnode, range, tmpDataset,
                                             &perSize);

  int tmpLeft = leftIdx;
  // create kInsertNewChildNumber new leaf nodes and store them in the node
  // array
  for (int i = 0; i < kInsertNewChildNumber; i++) {
    LeafNodeType tmpLeaf;
    std::vector<int> prefetchIndex(perSize[i].size);
    int s = perSize[i].left;
    int e = perSize[i].left + perSize[i].size;
    for (int j = s; j < e; j++) {
      double predictLeafIdx = root.model.PredictIdx(tmpDataset[j].first);
      int p = root.fetch_model.PrefetchPredict(predictLeafIdx);
      prefetchIndex[j - s] = p;
    }
    tmpLeaf.Init(tmpDataset, prefetchIndex, s, perSize[i].size, &data);
    if (isExternal) {
      tmpLeaf.m_left = tmpLeft;
      tmpLeft += perSize[i].size;
    }
    node.nodeArray[currnode.childLeft + i].cfArray =
        *(reinterpret_cast<CFArrayType<KeyType, ValueType> *>(&tmpLeaf));
  }

  // if the original leaf node is the cf array leaf node, we need to update the
  // pointer to the siblings of the new leaf nodes
  if (!isExternal) {
    if (previousIdx >= 0) {
      node.nodeArray[previousIdx].cfArray.nextLeaf = currnode.childLeft;
    }
    node.nodeArray[currnode.childLeft].cfArray.previousLeaf = previousIdx;
    int end = currnode.childLeft + kInsertNewChildNumber - 1;
    for (int i = currnode.childLeft + 1; i < end; i++) {
      node.nodeArray[i].cfArray.previousLeaf = i - 1;
      node.nodeArray[i].cfArray.nextLeaf = i + 1;
    }
    node.nodeArray[end].cfArray.previousLeaf = end - 1;
    if (nextIdx != -1) {
      node.nodeArray[end].cfArray.nextLeaf = nextIdx;
      node.nodeArray[nextIdx].cfArray.previousLeaf = end;
    }
  }
}

#endif  // FUNC_SPLIT_FUNCTION_H_