/**
 * @file minor_function.h
 * @author Jiaoyi
 * @brief the minor functions for constructing CARMI
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_MINOR_FUNCTION_H_
#define SRC_INCLUDE_CONSTRUCT_MINOR_FUNCTION_H_
#include <algorithm>
#include <utility>
#include <vector>

#include "../carmi.h"

template <typename KeyType, typename ValueType>
double CARMI<KeyType, ValueType>::CalculateEntropy(
    int size, int childNum, const std::vector<IndexPair> &perSize) const {
  double entropy = 0.0;
  if (size == 0) {
    return DBL_MAX;
  }
  for (int i = 0; i < childNum; i++) {
    auto p = static_cast<float>(perSize[i].size) / size;
    if (p != 0) entropy += p * (-log2(p));
  }
  return entropy;
}

template <typename KeyType, typename ValueType>
inline double CARMI<KeyType, ValueType>::CalculateCFArrayCost(
    int size, int totalSize, int givenBlockNum) {
  double space = kBaseNodeSpace;
  double time = carmi_params::kLeafBaseTime;
  if (givenBlockNum * CFArrayType<KeyType, ValueType>::kMaxBlockCapacity >=
      size) {
    // can be prefetched
    space += givenBlockNum * carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
  } else {
    // cannot be prefetched
    int neededBlock = CFArrayType<KeyType, ValueType>::CalNeededBlockNum(size);
    space += neededBlock * carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
    time += carmi_params::kMemoryAccessTime;
  }
  time *= size * 1.0 / totalSize;
  double cost = time + lambda * space;
  return cost;
}

template <typename KeyType, typename ValueType>
template <typename InnerNodeType>
void CARMI<KeyType, ValueType>::NodePartition(
    const InnerNodeType &currnode, const IndexPair &range,
    const DataVectorType &dataset, std::vector<IndexPair> *subData) const {
  int end = range.left + range.size;
  for (int i = range.left; i < end; i++) {
    int p = currnode.Predict(dataset[i].first);
    if ((*subData)[p].left == -1) {
      (*subData)[p].left = i;
    }
    (*subData)[p].size++;
  }
}

template <typename KeyType, typename ValueType>
template <typename InnerNodeType>
InnerNodeType CARMI<KeyType, ValueType>::InnerDivideAll(
    int c, const DataRange &range, SubDataset *subDataset) {
  InnerNodeType currnode(c);
  int l = range.initRange.left;
  int r = range.initRange.left + range.initRange.size;
  DataVectorType nowDataset(initDataset.begin() + l, initDataset.begin() + r);
  currnode.Train(range.initRange.left, range.initRange.size, initDataset);

  NodePartition<InnerNodeType>(currnode, range.initRange, initDataset,
                               &(subDataset->subInit));
  subDataset->subFind = subDataset->subInit;
  NodePartition<InnerNodeType>(currnode, range.insertRange, insertQuery,
                               &(subDataset->subInsert));
  return currnode;
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::UpdateLeaf() {
  if (isPrimary) return;
  node.nodeArray[scanLeaf[0]].cfArray.nextLeaf = scanLeaf[1];
  int end = scanLeaf.size() - 1;
  node.nodeArray[end].cfArray.nextLeaf = -1;
  node.nodeArray[end].cfArray.previousLeaf = scanLeaf[end - 1];
  for (int i = 1; i < end; i++) {
    node.nodeArray[scanLeaf[i]].cfArray.nextLeaf = scanLeaf[i + 1];
    node.nodeArray[scanLeaf[i]].cfArray.previousLeaf = scanLeaf[i - 1];
  }

  std::vector<int>().swap(scanLeaf);
}

template <typename KeyType, typename ValueType>
double CARMI<KeyType, ValueType>::CalculateFrequencyWeight(
    const DataRange &dataRange) {
  float frequency = 0.0;
  int findEnd = dataRange.findRange.left + dataRange.findRange.size;
  for (int l = dataRange.findRange.left; l < findEnd; l++)
    frequency += findQuery[l].second;
  int insertEnd = dataRange.insertRange.left + dataRange.insertRange.size;
  for (int l = dataRange.insertRange.left; l < insertEnd; l++)
    frequency += insertQuery[l].second;
  double frequency_weight = frequency / querySize;
  return frequency_weight;
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::ConstructEmptyNode(const DataRange &range) {
  BaseNode<KeyType, ValueType> optimal_node_struct;
  if (isPrimary) {
    ExternalArray<KeyType> tmp;
    tmp.Train(range.initRange.left, range.initRange.size, initDataset);
    optimal_node_struct.externalArray = tmp;
  } else {
    optimal_node_struct.cfArray = CFArrayType<KeyType, ValueType>();
  }
  structMap.insert({range.initRange, optimal_node_struct});
}

#endif  // SRC_INCLUDE_CONSTRUCT_MINOR_FUNCTION_H_
