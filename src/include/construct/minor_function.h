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

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
double CARMI<KeyType, ValueType, Compare, Alloc>::CalculateFrequencyWeight(
    const DataRange &dataRange) {
  float frequency = 0.0;
  // count the frequency of findQuery
  int findEnd = dataRange.findRange.left + dataRange.findRange.size;
  for (int i = dataRange.findRange.left; i < findEnd; i++)
    frequency += findQuery[i].second;
  // count the frequency  of insertQuery
  frequency += dataRange.insertRange.size;
  // calculate the weighted frequency of this sub-dataset
  double frequency_weight = frequency / querySize;
  return frequency_weight;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
double CARMI<KeyType, ValueType, Compare, Alloc>::CalculateEntropy(
    const std::vector<IndexPair> &perSize) const {
  // the sum of -size(i)*log(size(i))
  double slogs = 0.0;
  // the total size of the dataset
  int n = 0;
  for (int i = 0; i < perSize.size(); i++) {
    n += perSize[i].size;
    if (perSize[i].size != 0)
      slogs += static_cast<double>(perSize[i].size) * (-log2(perSize[i].size));
  }
  if (n == 0) {
    return -DBL_MAX;
  }

  double entropy = slogs / n + log2(n);
  return entropy;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
std::vector<double> CARMI<KeyType, ValueType, Compare,
                          Alloc>::CalculateCFArrayCost(int size,
                                                       int totalPrefetchedNum) {
  std::vector<double> cost(
      CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxBlockNum, 0);
  for (int k = 0;
       k < CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxBlockNum; k++) {
    double space = kBaseNodeSpace;
    double time = carmi_params::kLeafBaseTime;
    if ((k + 1) * CFArrayType<KeyType, ValueType, Compare,
                              Alloc>::kMaxBlockCapacity >=
        size) {
      // Case 1: these data points can be prefetched, then the space cost is the
      // space cost of allocated data blocks, and the time cost does not
      // increase
      space += (k + 1) * carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
    } else {
      // Case 2: these data points cannot be prefetched, then the space cost is
      // the space cost of actually needed data blocks and the time cost should
      // include the latency of a memory access
      int neededBlock =
          CFArrayType<KeyType, ValueType, Compare, Alloc>::CalNeededBlockNum(
              size);
      space += static_cast<double>(neededBlock) *
               carmi_params::kMaxLeafNodeSize / 1024.0 / 1024.0;
      time += carmi_params::kMemoryAccessTime;
    }
    time *= static_cast<double>(size) / totalPrefetchedNum;
    cost[k] = time + lambda * space;
  }
  return cost;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
template <typename InnerNodeType>
void CARMI<KeyType, ValueType, Compare, Alloc>::NodePartition(
    const InnerNodeType &currnode, const IndexPair &range,
    const DataVectorType &dataset, std::vector<IndexPair> *subData) const {
  int end = range.left + range.size;
  for (int i = range.left; i < end; i++) {
    int p = currnode.Predict(dataset[i].first);
    if (p < 0 || p >= (*subData).size()) {
      throw std::out_of_range(
          "CARMI::NodePartition: the output of the model is out of range.");
    }

    // if this sub-dataset is newly divided, store its leaf index in the dataset
    if ((*subData)[p].left == -1) {
      (*subData)[p].left = i;
    }
    // count the size of this sub-dataset
    (*subData)[p].size++;
  }
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
template <typename InnerNodeType>
void CARMI<KeyType, ValueType, Compare, Alloc>::NodePartition(
    const InnerNodeType &currnode, const IndexPair &range,
    const KeyVectorType &dataset, std::vector<IndexPair> *subData) const {
  int end = range.left + range.size;
  for (int i = range.left; i < end; i++) {
    int p = currnode.Predict(dataset[i]);

    // if this sub-dataset is newly divided, store its leaf index in the dataset
    if ((*subData)[p].left == -1) {
      (*subData)[p].left = i;
    }
    // count the size of this sub-dataset
    (*subData)[p].size++;
  }
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
template <typename InnerNodeType>
InnerNodeType CARMI<KeyType, ValueType, Compare, Alloc>::InnerDivideAll(
    const DataRange &range, int c, SubDataset *subDataset) {
  InnerNodeType currnode(c);
  int s = range.initRange.left;
  int e = range.initRange.size + s;
  DataVectorType tmpDataset(initDataset.begin() + s, initDataset.begin() + e);
  if (range.insertRange.size > 0) {
    s = range.insertRange.left;
    e = s + range.insertRange.size;
    for (int j = s; j < e; j++) {
      tmpDataset.push_back({insertQuery[j], static_cast<ValueType>(DBL_MAX)});
    }
    std::sort(tmpDataset.begin(), tmpDataset.end());
  }
  currnode.Train(0, tmpDataset.size(), tmpDataset);
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

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
void CARMI<KeyType, ValueType, Compare, Alloc>::UpdateLeaf() {
  if (isPrimary) return;
  node.nodeArray[scanLeaf[0]].cfArray.nextLeaf = scanLeaf[1];
  int end = scanLeaf.size() - 1;
  node.nodeArray[scanLeaf[end]].cfArray.nextLeaf = -1;
  node.nodeArray[scanLeaf[end]].cfArray.previousLeaf = scanLeaf[end - 1];
  for (int i = 1; i < end; i++) {
    node.nodeArray[scanLeaf[i]].cfArray.nextLeaf = scanLeaf[i + 1];
    node.nodeArray[scanLeaf[i]].cfArray.previousLeaf = scanLeaf[i - 1];
  }

  std::vector<int>().swap(scanLeaf);
}

#endif  // CONSTRUCT_MINOR_FUNCTION_H_
