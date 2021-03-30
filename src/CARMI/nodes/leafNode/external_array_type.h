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
#ifndef SRC_CARMI_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
#define SRC_CARMI_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_

#include <vector>

#include "../../../params.h"
#include "../../carmi.h"
#include "../../construct/minor_function.h"
#include "./leaf_nodes.h"

inline void CARMI::Train(int start_idx, int size, const DataVectorType &dataset,
                         ExternalArray *ext) {
  DataVectorType data = SetY(start_idx, size, dataset);
  if (size == 0) return;

  if ((ext->flagNumber & 0x00FFFFFF) != size) {
    ext->flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + size;
  }
  LRTrain(0, size, data, &(ext->theta1), &(ext->theta2));
  FindOptError<ExternalArray>(0, size, data, ext);
}

/**
 * @brief initialize external array node
 *
 * @param cap the capacity of this leaf node
 * @param start_idx the start index of data points
 * @param size the size of data points
 * @param dataset
 * @param ext leaf node
 */
inline void CARMI::Init(int cap, int start_idx, int size,
                        const DataVectorType &dataset, ExternalArray *ext) {
  ext->m_left = start_idx;
  if (size == 0) return;

  Train(start_idx, size, dataset, ext);
}

#endif  // SRC_CARMI_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
