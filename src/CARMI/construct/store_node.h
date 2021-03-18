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
TYPE CARMI::StoreInnerNode(const IndexPair &range, TYPE *node) {
  int optimalChildNumber = node->flagNumber & 0x00FFFFFF;
  SubDataset subDataset(optimalChildNumber);

  NodePartition<TYPE>(*node, range, initDataset, &(subDataset.subInit));
  node->childLeft = AllocateChildMemory(optimalChildNumber);

  for (int i = 0; i < optimalChildNumber; i++) {
    DataRange subRange(subDataset.subInit[i], subDataset.subFind[i],
                       subDataset.subInsert[i]);
    StoreOptimalNode(node->childLeft + i, subRange);
  }
  return *node;
}

/**
 * @brief store nodes
 *
 * @param storeIdx the index of this node being stored in entireChild
 * @param optimalType the type of this node
 * @param range the left and size of the data points in initDataset
 */
void CARMI::StoreOptimalNode(int storeIdx, const DataRange &range) {
  auto it = structMap.find(range.initRange);

#ifdef DEBUG
  if (it == structMap.end()) std::cout << "WRONG!" << std::endl;
#endif  // DEBUG

  int type = it->second.array.flagNumber >> 24;
  switch (type) {
    case LR_INNER_NODE: {
      StoreInnerNode<LRModel>(range.initRange, &(it->second.lr));
      entireChild[storeIdx].lr = it->second.lr;
      break;
    }
    case PLR_INNER_NODE: {
      StoreInnerNode<PLRModel>(range.initRange, &(it->second.plr));
      entireChild[storeIdx].plr = it->second.plr;
      break;
    }
    case HIS_INNER_NODE: {
      StoreInnerNode<HisModel>(range.initRange, &(it->second.his));
      entireChild[storeIdx].his = it->second.his;
      break;
    }
    case BS_INNER_NODE: {
      StoreInnerNode<BSModel>(range.initRange, &(it->second.bs));
      entireChild[storeIdx].bs = it->second.bs;
      break;
    }
    case ARRAY_LEAF_NODE: {
      ArrayType node = it->second.array;
      StoreData(node.m_capacity, range.initRange.left, range.initRange.size,
                initDataset, &node);
      entireChild[storeIdx].array = node;
      if (range.initRange.size > 0)
        scanLeaf.insert({initDataset[range.initRange.left].first, storeIdx});
      break;
    }
    case GAPPED_ARRAY_LEAF_NODE: {
      GappedArrayType node = it->second.ga;
      StoreData(node.capacity, range.initRange.left, range.initRange.size,
                initDataset, &node);
      entireChild[storeIdx].ga = node;
      if (range.initRange.size > 0)
        scanLeaf.insert({initDataset[range.initRange.left].first, storeIdx});
      break;
    }
    case EXTERNAL_ARRAY_LEAF_NODE: {
      ExternalArray node = it->second.externalArray;
      node.flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + range.initRange.size;
      node.m_left = range.initRange.left;
      entireChild[storeIdx].externalArray = node;
      break;
    }
  }
}

#endif  // SRC_CARMI_CONSTRUCT_STORE_NODE_H_
