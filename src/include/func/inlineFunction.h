/**
 * @file inlineFunction.h
 * @author Jiaoyi
 * @brief the inline functions for public functions
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef FUNC_INLINEFUNCTION_H_
#define FUNC_INLINEFUNCTION_H_

#include <float.h>

#include <algorithm>
#include <vector>

#include "../carmi.h"
#include "../construct/minor_function.h"
#include "../params.h"

template <typename KeyType, typename ValueType>
template <typename LeafNodeType>
inline void CARMI<KeyType, ValueType>::Split(bool isExternal, int left,
                                             int size, int previousIdx,
                                             int idx) {
  int actualSize = 0;
  DataVectorType tmpDataset;
  if (isExternal) {
    tmpDataset = ExternalArray<KeyType>::ExtractDataset(
        external_data, left, size + left, recordLength, &actualSize);
  } else {
    tmpDataset = CFArrayType<KeyType, ValueType>::ExtractDataset(
        data, left, size + left, &actualSize);
  }

  int nextIdx = node.nodeArray[idx].cfArray.nextLeaf;

  // create a new inner node
  auto currnode = LRModel<KeyType, ValueType>(kInsertNewChildNumber);
  currnode.childLeft = node.AllocateNodeMemory(kInsertNewChildNumber);
  currnode.Train(0, actualSize, tmpDataset);
  node.nodeArray[idx].lr = currnode;

  std::vector<IndexPair> perSize(kInsertNewChildNumber, emptyRange);
  IndexPair range{0, actualSize};
  NodePartition<LRModel<KeyType, ValueType>>(currnode, range, tmpDataset,
                                             &perSize);

  int tmpLeft = left;
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
    tmpLeaf.Init(tmpDataset, prefetchIndex, perSize[i].left, perSize[i].size,
                 &data);
    if (isExternal) {
      tmpLeaf.m_left = tmpLeft;
      tmpLeft += perSize[i].size;
    }
    node.nodeArray[currnode.childLeft + i].cfArray =
        *(reinterpret_cast<CFArrayType<KeyType, ValueType> *>(&tmpLeaf));
  }

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

#endif  // FUNC_INLINEFUNCTION_H_
