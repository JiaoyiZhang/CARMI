/**
 * @file store_node.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_CONSTRUCT_STORE_NODE_H_
#define SRC_CARMI_CONSTRUCT_STORE_NODE_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include "../../params.h"
#include "../carmi.h"
#include "../dataManager/child_array.h"
#include "../dataManager/datapoint.h"
#include "../nodes/innerNode/bs_model.h"
#include "../nodes/innerNode/his_model.h"
#include "../nodes/innerNode/lr_model.h"
#include "../nodes/innerNode/plr_model.h"
#include "../nodes/leafNode/array_type.h"
#include "../nodes/leafNode/external_array_type.h"
#include "../nodes/leafNode/ga_type.h"
#include "./dp_inner.h"

/**
 * @brief store an inner node
 *
 * @tparam TYPE the type of this node
 * @param range the left and size of the data points in initDataset
 * @return TYPE trained node
 */
template <typename TYPE>
TYPE CARMI::StoreInnerNode(const DataRange &range) {
  auto it = structMap.find(range.initRange);

#ifdef DEBUG
  if (it == structMap.end()) std::cout << "WRONG!" << std::endl;
#endif  // DEBUG

  TYPE node = *(reinterpret_cast<TYPE *>(&it->second));
  int optimalChildNumber = node.flagNumber & 0x00FFFFFF;
  SubDataset subDataset(optimalChildNumber);

  NodePartition<TYPE>(node, range.initRange, initDataset,
                      &(subDataset.subInit));
  node.childLeft = AllocateChildMemory(optimalChildNumber);

  for (int i = 0; i < optimalChildNumber; i++) {
    auto iter = structMap.find(subDataset.subInit[i]);
    int type = iter->second.array.flagNumber >> 24;
    DataRange subRange(subDataset.subInit[i], subDataset.subFind[i],
                       subDataset.subInsert[i]);
    StoreOptimalNode(node.childLeft + i, type, subRange);
  }
  return node;
}

/**
 * @brief store nodes
 *
 * @param storeIdx the index of this node being stored in entireChild
 * @param optimalType the type of this node
 * @param range the left and size of the data points in initDataset
 */
void CARMI::StoreOptimalNode(int storeIdx, int optimalType,
                             const DataRange &range) {
//   if (range.initRange.size == 0) {
//     if (kPrimaryIndex) {
//       ExternalArray node;
//       InitExternalArray(&node, range.initRange.left, range.initRange.size);
//       entireChild[storeIdx].externalArray = node;
//     } else {
//       GappedArrayType node(kThreshold);
//       InitGA(node.capacity, range.initRange.left, range.initRange.size,
//              initDataset, &node);
//       entireChild[storeIdx].ga = node;
//     }
// #ifdef DEBUG
//     if (optimalType < ARRAY_LEAF_NODE)
//       std::cout << "WRONG! size==0, type is:" << optimalType << std::endl;
// #endif  // DEBUG
//     return;
//   }

  auto it = structMap.find(range.initRange);

  switch (optimalType) {
    case LR_INNER_NODE: {
      auto node = StoreInnerNode<LRModel>(range);
      entireChild[storeIdx].lr = node;
      break;
    }
    case PLR_INNER_NODE: {
      auto node = StoreInnerNode<PLRModel>(range);
      entireChild[storeIdx].plr = node;
      break;
    }
    case HIS_INNER_NODE: {
      auto node = StoreInnerNode<HisModel>(range);
      entireChild[storeIdx].his = node;
      break;
    }
    case BS_INNER_NODE: {
      auto node = StoreInnerNode<BSModel>(range);
      entireChild[storeIdx].bs = node;
      break;
    }
    case ARRAY_LEAF_NODE: {
      ArrayType node = *(reinterpret_cast<ArrayType *>(&it->second));
      UpdatePara(std::max(range.initRange.size, kThreshold),
                 range.initRange.size, &node);
      StoreData(range.initRange.left, range.initRange.size, initDataset, &node);
      entireChild[storeIdx].array = node;
      if (range.initRange.size > 0)
        scanLeaf.insert({initDataset[range.initRange.left].first, storeIdx});
      break;
    }
    case GAPPED_ARRAY_LEAF_NODE: {
      GappedArrayType node =
          *(reinterpret_cast<GappedArrayType *>(&it->second));  // .ga
      UpdatePara(std::max(range.initRange.size, kThreshold),
                 range.initRange.size, &node);
      StoreData(range.initRange.left, range.initRange.size, initDataset, &node);
      entireChild[storeIdx].ga = node;
      if (range.initRange.size > 0)
        scanLeaf.insert({initDataset[range.initRange.left].first, storeIdx});
      break;
    }
    case EXTERNAL_ARRAY_LEAF_NODE: {
      ExternalArray node = *(reinterpret_cast<ExternalArray *>(&it->second));
      node.flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + range.initRange.size;
      node.m_left = range.initRange.left;
      entireChild[storeIdx].externalArray = node;
      break;
    }
  }
}

#endif  // SRC_CARMI_CONSTRUCT_STORE_NODE_H_
