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
#ifndef CONSTRUCT_MINOR_FUNCTION_H_
#define CONSTRUCT_MINOR_FUNCTION_H_
#include <algorithm>
#include <utility>
#include <vector>

#include "../carmi.h"

template <typename KeyType, typename ValueType>
double CARMI<KeyType, ValueType>::CalculateFrequencyWeight(
    const DataRange &dataRange) {
  float frequency = 0.0;
  // count the frequency of findQuery
  int findEnd = dataRange.findRange.left + dataRange.findRange.size;
  for (int i = dataRange.findRange.left; i < findEnd; i++)
    frequency += findQuery[i].second;
  // count the frequency  of insertQuery
  int insertEnd = dataRange.insertRange.left + dataRange.insertRange.size;
  for (int i = dataRange.insertRange.left; i < insertEnd; i++)
    frequency += insertQuery[i].second;
  // calculate the weighted frequency of this sub-dataset
  double frequency_weight = frequency / querySize;
  return frequency_weight;
}

template <typename KeyType, typename ValueType>
double CARMI<KeyType, ValueType>::CalculateEntropy(
    const std::vector<IndexPair> &perSize) const {
  // the sum of -size(i)*log(size(i))
  double slogs = 0.0;
  // the total size of the dataset
  int n = 0;
  for (int i = 0; i < perSize.size(); i++) {
    n += perSize[i].size;
    if (perSize[i].size != 0)
      slogs += perSize[i].size * 1.0 * (-log2(perSize[i].size));
  }
  if (n == 0) {
    return DBL_MAX;
  }

  double entropy = slogs / n + log2(n);
  return entropy;
}

template <typename KeyType, typename ValueType>
std::vector<double> CARMI<KeyType, ValueType>::CalculateCFArrayCost(
    int size, int totalPrefetchedNum) {
  std::vector<double> cost(CFArrayType<KeyType, ValueType>::kMaxBlockNum, 0);
  for (int k = 0; k < CFArrayType<KeyType, ValueType>::kMaxBlockNum; k++) {
    double space = kBaseNodeSpace;
    double time = carmi_params::kLeafBaseTime;
    if ((k + 1) * CFArrayType<KeyType, ValueType>::kMaxBlockCapacity >= size) {
      // Case 1: these data points can be prefetched, then the space cost is the
      // space cost of allocated data blocks and the time cost does not increase
      space += (k + 1) * carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
    } else {
      // Case 2: these data points cannot be prefetched, then the space cost is
      // the space cost of actually needed data blocks and the time cost should
      // include the latency of a memory access
      int neededBlock =
          CFArrayType<KeyType, ValueType>::CalNeededBlockNum(size);
      space += neededBlock * carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
      time += carmi_params::kMemoryAccessTime;
    }
    time *= size * 1.0 / totalPrefetchedNum;
    cost[k] = time + lambda * space;
  }
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
    // if this sub-dataset is newly divided, store its leaf index in the dataset
    if ((*subData)[p].left == -1) {
      (*subData)[p].left = i;
    }
    // count the size of this sub-dataset
    (*subData)[p].size++;
  }
}

template <typename KeyType, typename ValueType>
template <typename InnerNodeType>
InnerNodeType CARMI<KeyType, ValueType>::InnerDivideAll(
    const DataRange &range, int c, SubDataset *subDataset) {
  InnerNodeType currnode(c);
  currnode.Train(range.initRange.left, range.initRange.size, initDataset);
  // split initDataset into c sub-datasets
  NodePartition<InnerNodeType>(currnode, range.initRange, initDataset,
                               &(subDataset->subInit));
  // split findQuery into c sub-datasets
  subDataset->subFind = subDataset->subInit;
  // split insertQuery into c sub-datasets
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

#endif  // CONSTRUCT_MINOR_FUNCTION_H_
