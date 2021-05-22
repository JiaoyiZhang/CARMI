/**
 * @file minor_function.h
 * @author Jiaoyi
 * @brief the minor functions for constructing CARMI
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_MINOR_FUNCTION_H_
#define SRC_INCLUDE_CONSTRUCT_MINOR_FUNCTION_H_
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
template <typename TYPE>
void CARMI<KeyType, ValueType>::NodePartition(
    const TYPE &node, const IndexPair &range, const DataVectorType &dataset,
    std::vector<IndexPair> *subData) const {
  int end = range.left + range.size;
  for (int i = range.left; i < end; i++) {
    int p = node.Predict(dataset[i].first);
    if ((*subData)[p].left == -1) {
      (*subData)[p].left = i;
    }
    (*subData)[p].size++;
  }
}

template <typename KeyType, typename ValueType>
template <typename TYPE>
TYPE CARMI<KeyType, ValueType>::InnerDivideAll(int c, const DataRange &range,
                                               SubDataset *subDataset) {
  TYPE node;
  node.SetChildNumber(c);
  Train(range.initRange.left, range.initRange.size, initDataset, &node);

  NodePartition<TYPE>(node, range.initRange, initDataset,
                      &(subDataset->subInit));
  subDataset->subFind = subDataset->subInit;
  NodePartition<TYPE>(node, range.insertRange, insertQuery,
                      &(subDataset->subInsert));
  return node;
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::UpdateLeaf() {
  if (isPrimary) return;
  entireChild[scanLeaf[0]].array.nextLeaf = scanLeaf[1];
  int end = scanLeaf.size() - 1;
  entireChild[end].array.nextLeaf = -1;
  entireChild[end].array.previousLeaf = scanLeaf[end - 1];
  for (int i = 1; i < end; i++) {
    entireChild[scanLeaf[i]].array.nextLeaf = scanLeaf[i + 1];
    entireChild[scanLeaf[i]].array.previousLeaf = scanLeaf[i - 1];
  }

  // scanLeaf.clear();
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
  BaseNode optimal_node_struct;
  if (isPrimary) {
    ExternalArray tmp(kThreshold);
    Train(range.initRange.left, range.initRange.size, initDataset, &tmp);
    optimal_node_struct.externalArray = tmp;
  } else {
    GappedArrayType tmpNode(kThreshold);
    tmpNode.density = 0.5;
    Train(range.initRange.left, range.initRange.size, initDataset, &tmpNode);
    optimal_node_struct.ga = tmpNode;
  }
  structMap.insert({range.initRange, optimal_node_struct});
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::LRTrain(const int left, const int size,
                                        const DataVectorType &dataset, float *a,
                                        float *b) {
  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  int end = left + size;
  for (int i = left; i < end; i++) {
    t1 += dataset[i].first * dataset[i].first;
    t2 += dataset[i].first;
    t3 += dataset[i].first * dataset[i].second;
    t4 += dataset[i].second;
  }
  if (t1 * size - t2 * t2) {
    *a = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    *b = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
  } else {
    *a = 0;
    *b = 0;
  }
}

template <typename KeyType, typename ValueType>
typename CARMI<KeyType, ValueType>::DataVectorType
CARMI<KeyType, ValueType>::ExtractData(const int left, const int size,
                                       const DataVectorType &dataset,
                                       int *actual) {
  *actual = 0;
  DataVectorType data(size, {DBL_MIN, DBL_MIN});
  int end = left + size;
  for (int i = left; i < end; i++) {
    if (dataset[i].first != DBL_MIN) {
      data[(*actual)++] = dataset[i];
    }
  }
  return data;
}

template <typename KeyType, typename ValueType>
typename CARMI<KeyType, ValueType>::DataVectorType
CARMI<KeyType, ValueType>::SetY(const int left, const int size,
                                const DataVectorType &dataset) {
  DataVectorType data(size, {DBL_MIN, DBL_MIN});
  int end = left + size;
  for (int i = left, j = 0; i < end; i++, j++) {
    data[j].first = dataset[i].first;
    data[j].second = static_cast<double>(j) / size;
  }
  return data;
}

template <typename KeyType, typename ValueType>
template <typename TYPE>
void CARMI<KeyType, ValueType>::FindOptError(int start_idx, int size,
                                             const DataVectorType &dataset,
                                             TYPE *node) {
  std::vector<int> error_count(size + 1, 0);

  // record each difference
  int p, d;
  int end = start_idx + size;
  for (int i = start_idx; i < end; i++) {
    p = node->Predict(dataset[i].first);
    d = abs(i - start_idx - p);

    error_count[d]++;
  }

  // find the optimal value of error
  int minRes = size * log2(size);
  int res;
  int cntBetween = 0;
  for (int e = 0; e <= size; e++) {
    if (error_count[e] == 0) {
      continue;
    }
    cntBetween += error_count[e];
    if (e != 0)
      res = cntBetween * log2(e) + (size - cntBetween) * log2(size);
    else
      res = (size - cntBetween) * log2(size);
    if (res < minRes) {
      minRes = res;
      node->error = e;
    }
  }
}
#endif  // SRC_INCLUDE_CONSTRUCT_MINOR_FUNCTION_H_
