/**
 * @file array_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_NODES_LEAFNODE_ARRAY_TYPE_H_
#define SRC_INCLUDE_NODES_LEAFNODE_ARRAY_TYPE_H_

#include <float.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "../../carmi.h"
#include "../../construct/minor_function.h"
#include "../../params.h"
#include "./leaf_nodes.h"

inline int ArrayType::Predict(double key) const {
  // return the predicted idx in the leaf node
  int size = (flagNumber & 0x00FFFFFF);
  int p = (theta1 * key + theta2) * size;
  if (p < 0)
    p = 0;
  else if (p >= size && size != 0)
    p = size - 1;
  return p;
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Init(int cap, int left, int size,
                                            const DataVectorType &dataset,
                                            ArrayType *arr) {
  if (size == 0) return;
  int actualSize = 0;
  DataVectorType newDataset = ExtractData(left, size, dataset, &actualSize);

  Train(0, actualSize, newDataset, arr);
  StoreData(cap, 0, actualSize, newDataset, arr);
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::StoreData(int cap, int left, int size,
                                                 const DataVectorType &dataset,
                                                 ArrayType *arr) {
  if (arr->m_left != -1) {
    ReleaseMemory(arr->m_left, arr->m_capacity);
  }
  if (cap < size) {
    arr->m_capacity = GetActualSize(size);
  } else {
    arr->m_capacity = cap;
  }
  if (arr->m_capacity == size) {
    arr->m_capacity =
        GetActualSize(std::min(size + 1, carmi_params::kLeafMaxCapacity));
  }
  arr->m_left = AllocateMemory(arr->m_capacity);

  int end = left + size;
  for (int i = arr->m_left, j = left; j < end; i++, j++)
    entireData[i] = dataset[j];
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Train(int start_idx, int size,
                                             const DataVectorType &dataset,
                                             ArrayType *arr) {
  if (size == 0) return;

  DataVectorType data = SetY(start_idx, size, dataset);
  arr->flagNumber = (ARRAY_LEAF_NODE << 24) + size;
  LRTrain(0, size, data, &(arr->theta1), &(arr->theta2));
  FindOptError<ArrayType>(0, size, data, arr);
}

#endif  // SRC_INCLUDE_NODES_LEAFNODE_ARRAY_TYPE_H_