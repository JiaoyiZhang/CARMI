/**
 * @file ga_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_NODES_LEAFNODE_GA_TYPE_H_
#define SRC_INCLUDE_NODES_LEAFNODE_GA_TYPE_H_

#include <float.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "../../carmi.h"
#include "../../construct/minor_function.h"
#include "./leaf_nodes.h"

/**
 * @brief predict the position of the given key
 *
 * @param key
 * @return int the predicted index in the leaf node
 */
inline int GappedArrayType::Predict(double key) const {
  // return the predicted idx in the leaf node
  int p = (theta1 * key + theta2) * maxIndex;
  if (p < 0)
    p = 0;
  else if (p >= maxIndex)
    p = maxIndex;
  return p;
}

/**
 * @brief initialize gapped array node
 *
 * @param cap the capacity of this leaf node
 * @param left the start index of data points
 * @param size  the size of data points
 * @param subDataset
 * @param ga leaf node
 */
template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Init(int cap, int left, int size,
                                            const DataVectorType &subDataset,
                                            GappedArrayType *ga) {
  if (size == 0) return;
  int actualSize = 0;
  DataVectorType newDataset = ExtractData(left, size, subDataset, &actualSize);

  Train(0, actualSize, newDataset, ga);
  StoreData(cap, 0, actualSize, newDataset, ga);
}

/**
 * @brief store data points into the entireData
 *
 * @param cap the capacity of this leaf node
 * @param left the start index of data points
 * @param size  the size of data points
 * @param subDataset
 * @param ga leaf node
 */
template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::StoreData(int cap, int left, int size,
                                                 const DataVectorType &dataset,
                                                 GappedArrayType *ga) {
  if (ga->m_left != -1) {
    ReleaseMemory(ga->m_left, ga->capacity);
  }
  if ((static_cast<float>(size) / cap > ga->density))
    ga->capacity = static_cast<float>(cap) / ga->density;
  if (ga->capacity > carmi_params::kLeafMaxCapacity) {
    ga->capacity = carmi_params::kLeafMaxCapacity;
  }
  ga->capacity = GetActualSize(ga->capacity);
  ga->m_left = AllocateMemory(ga->capacity);

  int end = left + size;

  if (size > carmi_params::kLeafMaxCapacity * 0.95) {
    for (int i = left, j = ga->m_left; i < end; i++) {
      entireData[j++] = dataset[i];
    }
    ga->maxIndex = size - 1;
    return;
  }

  int k = ga->density / (1 - ga->density);
  float rate = static_cast<float>(size) / ga->capacity;
  if (rate > ga->density) {
    k = rate / (1 - rate);
  }
  int cnt = 0;
  int j = ga->m_left;
  for (int i = left; i < end; i++) {
    if (cnt >= k) {
      entireData[j++] = {DBL_MIN, DBL_MIN};
      cnt = 0;
      if (j > ga->m_left + 2048) {
        k += 2;
      }
    }
    cnt++;
    entireData[j++] = dataset[i];
    ga->maxIndex = j - 1 - ga->m_left;
  }
}

/**
 * @brief train the ga node
 *
 * @param start_idx the start index of data points
 * @param size the size of data points
 * @param dataset
 * @param ga leaf node
 */
template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Train(int start_idx, int size,
                                             const DataVectorType &dataset,
                                             GappedArrayType *ga) {
  if ((ga->flagNumber & 0x00FFFFFF) != size) {
    ga->flagNumber = (GAPPED_ARRAY_LEAF_NODE << 24) + size;
  }

  if (size == 0) return;

  ga->maxIndex = size;

  DataVectorType data = SetY(start_idx, size, dataset);
  LRTrain(0, size, data, &(ga->theta1), &(ga->theta2));

  FindOptError<GappedArrayType>(0, size, data, ga);
  ga->error /= ga->density;
}

#endif  // SRC_INCLUDE_NODES_LEAFNODE_GA_TYPE_H_
