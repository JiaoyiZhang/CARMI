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
#include "../nodes/leafNode/ga_type.h"
#include "../nodes/leafNode/ycsb_leaf_type.h"
#include "./dp_inner.h"

template <typename TYPE>
TYPE CARMI::storeInnerNode(int storeIdx, const MapKey &key,
                           const DataRange &range) {
  auto it = structMap.find(key);
  if (it == structMap.end()) std::cout << "WRONG!" << std::endl;

  int optimalChildNumber = it->second.childNum;
  SubDataset subDataset(optimalChildNumber);
  TYPE node = InnerDivideAll<TYPE>(optimalChildNumber, range, &subDataset);
  node.childLeft = allocateChildMemory(optimalChildNumber);

  for (int i = 0; i < optimalChildNumber; i++) {
    auto nowKey = it->second.child[i];
    int type;
    auto iter = structMap.find(nowKey);
    if (iter == structMap.end())
      type = ARRAY_LEAF_NODE;
    else
      type = iter->second.type;
    DataRange subRange(subDataset.subInit[i], subDataset.subFind[i],
                       subDataset.subInsert[i]);
    storeOptimalNode(node.childLeft + i, type, nowKey, subRange);
  }
  return node;
}

void CARMI::storeOptimalNode(int storeIdx, int optimalType, const MapKey key,
                             const DataRange &range) {
  if (range.initRange.size == 0) {
    if (kPrimaryIndex) {
      YCSBLeaf node;
      initYCSB(&node, range.initRange.left, range.initRange.size);
      entireChild[storeIdx].ycsbLeaf = node;
    } else {
      GappedArrayType node(kThreshold);
      initGA(node.capacity, range.initRange.left, range.initRange.size,
             initDataset, &node);
      entireChild[storeIdx].ga = node;
    }
#ifdef DEBUG
    if (optimalType < ARRAY_LEAF_NODE)
      std::cout << "WRONG! size==0, type is:" << optimalType << std::endl;
#endif  // DEBUG
    return;
  }

  switch (optimalType) {
    case LR_INNER_NODE: {
      auto node = storeInnerNode<LRModel>(storeIdx, key, range);
      entireChild[storeIdx].lr = node;
      break;
    }
    case PLR_INNER_NODE: {
      auto node = storeInnerNode<PLRModel>(storeIdx, key, range);
      entireChild[storeIdx].plr = node;
      break;
    }
    case HIS_INNER_NODE: {
      auto node = storeInnerNode<HisModel>(storeIdx, key, range);
      entireChild[storeIdx].his = node;
      break;
    }
    case BS_INNER_NODE: {
      auto node = storeInnerNode<BSModel>(storeIdx, key, range);
      entireChild[storeIdx].bs = node;
      break;
    }
    case ARRAY_LEAF_NODE: {
      ArrayType node(std::max(range.initRange.size, kThreshold));
      initArray(node.m_capacity, range.initRange.left, range.initRange.size,
                initDataset, &node);
      entireChild[storeIdx].array = node;
      if (range.initRange.size > 0)
        scanLeaf.insert({initDataset[range.initRange.left].first, storeIdx});
      break;
    }
    case GAPPED_ARRAY_LEAF_NODE: {
      auto it = structMap.find(key);
      GappedArrayType node(kThreshold);
      node.density = it->second.density;
      initGA(node.capacity, range.initRange.left, range.initRange.size,
             initDataset, &node);
      entireChild[storeIdx].ga = node;
      if (range.initRange.size > 0)
        scanLeaf.insert({initDataset[range.initRange.left].first, storeIdx});
      break;
    }
    case EXTERNAL_ARRAY_LEAF_NODE: {
      YCSBLeaf node;
      initYCSB(&node, range.initRange.left, range.initRange.size);
      entireChild[storeIdx].ycsbLeaf = node;
      break;
    }
  }
}

#endif  // SRC_CARMI_CONSTRUCT_STORE_NODE_H_
