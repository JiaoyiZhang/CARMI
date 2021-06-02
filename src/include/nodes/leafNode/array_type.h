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

template <typename KeyType>
inline int ArrayType<KeyType>::Predict(double key) const {
  // return the idx of the union in entireData
  int start_idx = 0;
  int end_idx = std::max(0, (flagNumber & 0x00FFFFFF) - 2);
  if (slotkeys[0] == DBL_MIN || key < slotkeys[0]) {
    return 0;
  }
  if (key >= slotkeys[end_idx]) {
    return end_idx + 1;
  }

  int mid;
  while (start_idx < end_idx) {
    mid = (start_idx + end_idx) / 2;
    if (slotkeys[mid] < key)
      start_idx = mid + 1;
    else
      end_idx = mid;
  }
  if (slotkeys[start_idx] == key) {
    return start_idx + 1;
  }
  return start_idx;
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Init(int left, int size,
                                            const DataVectorType &dataset,
                                            ArrayType<KeyType> *arr) {
  if (size == 0) return;
  int actualSize = 0;
  DataVectorType newDataset = ExtractData(left, size, dataset, &actualSize);
  int neededLeafNum = GetActualSize(size);

  StoreData(neededLeafNum, 0, actualSize, newDataset, arr);
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Rebalance(const int unionleft,
                                                 const int unionright,
                                                 ArrayType<KeyType> *arr) {
  int actualSize = 0;
  DataVectorType newDataset = ExtractData(unionleft, unionright, &actualSize);
  int nowLeafNum = unionright - unionleft;

  StoreData(nowLeafNum, 0, actualSize, newDataset, arr);
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Expand(const int unionleft,
                                              const int unionright,
                                              ArrayType<KeyType> *arr) {
  int actualSize = 0;
  DataVectorType newDataset = ExtractData(unionleft, unionright, &actualSize);
  int neededLeafNum = unionright - unionleft + 1;

  StoreData(neededLeafNum, 0, actualSize, newDataset, arr);
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::StoreData(int neededLeafNum, int left,
                                                 int size,
                                                 const DataVectorType &dataset,
                                                 ArrayType<KeyType> *arr) {
  int nowLeafNum = arr->flagNumber & 0x00FFFFFF;
  if (nowLeafNum != neededLeafNum) {
    if (arr->m_left != -1) {
      ReleaseMemory(arr->m_left, nowLeafNum);
    }
    arr->m_left = AllocateMemory(neededLeafNum);
  }

  arr->flagNumber = (ARRAY_LEAF_NODE << 24) + 0;
  LeafSlots<KeyType, ValueType> tmp;
  int avg = std::max(1, size / neededLeafNum + 1);
  avg = std::min(avg, kMaxSlotNum);
  BaseNode<KeyType> tmpArr;

  int end = left + size;
  for (int i = arr->m_left, j = left, k = 1; j < end; j++, k++) {
    SlotsUnionInsert(dataset[j], 0, &tmp, &tmpArr);
    if (k == avg || j == end - 1) {
      k = 0;
#ifdef DEBUG
      CheckBound(i, 0, nowDataSize);
#endif  // DEBUG
      entireData[i++] = tmp;
      tmp = LeafSlots<KeyType, ValueType>();
      arr->flagNumber++;
    }
  }
  if (neededLeafNum == 1) {
    if (size > 0) {
      arr->slotkeys[0] = dataset[end - 1].first + 1;
    }
    return;
  }
  end = arr->m_left + neededLeafNum;
  int j = 0;
  for (int i = arr->m_left + 1; i < end; i++, j++) {
    arr->slotkeys[j] = entireData[i].slots[0].first;
  }
}

#endif  // SRC_INCLUDE_NODES_LEAFNODE_ARRAY_TYPE_H_
