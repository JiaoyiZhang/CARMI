/**
 * @file ycsb_leaf_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
#define SRC_INCLUDE_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_

#include <vector>

#include "../../carmi.h"
#include "../../construct/minor_function.h"
#include "../../params.h"
#include "./leaf_nodes.h"

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Train(int start_idx, int size,
                                             const DataVectorType &dataset,
                                             ExternalArray *ext) {
  DataVectorType data = SetY(start_idx, size, dataset);
  if (size == 0) return;

  if ((ext->flagNumber & 0x00FFFFFF) != size) {
    ext->flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + size;
  }
  LRTrain(0, size, data, &(ext->theta1), &(ext->theta2));
  FindOptError<ExternalArray>(0, size, data, ext);
}

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Init(int start_idx, int size,
                                            const DataVectorType &dataset,
                                            ExternalArray *ext) {
  ext->m_left = start_idx;
  if (size == 0) return;

  Train(start_idx, size, dataset, ext);
}

#endif  // SRC_INCLUDE_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
