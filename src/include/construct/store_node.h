/**
 * @file store_node.h
 * @author Jiaoyi
 * @brief store inner and leaf nodes
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_STORE_NODE_H_
#define SRC_INCLUDE_CONSTRUCT_STORE_NODE_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include "../carmi.h"
#include "../memoryLayout/node_array.h"
#include "../nodes/innerNode/bs_model.h"
#include "../nodes/innerNode/his_model.h"
#include "../nodes/innerNode/lr_model.h"
#include "../nodes/innerNode/plr_model.h"
#include "../nodes/leafNode/cfarray_type.h"
#include "../nodes/leafNode/external_array_type.h"
#include "../params.h"
#include "./dp_inner.h"

template <typename KeyType, typename ValueType>
template <typename TYPE>
TYPE CARMI<KeyType, ValueType>::StoreInnerNode(const IndexPair &range,
                                               TYPE *currnode) {
  int optimalChildNumber = currnode->flagNumber & 0x00FFFFFF;
  SubDataset subDataset(optimalChildNumber);

  NodePartition<TYPE>(*currnode, range, initDataset, &(subDataset.subInit));
  currnode->childLeft = node.AllocateNodeMemory(optimalChildNumber);

  for (int i = 0; i < optimalChildNumber; i++) {
    DataRange subRange(subDataset.subInit[i], subDataset.subFind[i],
                       subDataset.subInsert[i]);
    StoreOptimalNode(currnode->childLeft + i, subRange);
  }
  return *currnode;
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::StoreOptimalNode(int storeIdx,
                                                 const DataRange &range) {
  auto it = structMap.find(range.initRange);

  int type = it->second.cfArray.flagNumber >> 24;
  switch (type) {
    case LR_INNER_NODE: {
      StoreInnerNode<LRModel<KeyType, ValueType>>(range.initRange,
                                                  &(it->second.lr));
      node.nodeArray[storeIdx].lr = it->second.lr;
      break;
    }
    case PLR_INNER_NODE: {
      StoreInnerNode<PLRModel<KeyType, ValueType>>(range.initRange,
                                                   &(it->second.plr));
      node.nodeArray[storeIdx].plr = it->second.plr;
      break;
    }
    case HIS_INNER_NODE: {
      StoreInnerNode<HisModel<KeyType, ValueType>>(range.initRange,
                                                   &(it->second.his));
      node.nodeArray[storeIdx].his = it->second.his;
      break;
    }
    case BS_INNER_NODE: {
      StoreInnerNode<BSModel<KeyType, ValueType>>(range.initRange,
                                                  &(it->second.bs));
      node.nodeArray[storeIdx].bs = it->second.bs;
      break;
    }
    case ARRAY_LEAF_NODE: {
      remainingNode.push_back(storeIdx);
      remainingRange.push_back(range);
      break;
    }
    case EXTERNAL_ARRAY_LEAF_NODE: {
      ExternalArray<KeyType> currnode = it->second.externalArray;
      int size = range.initRange.size;
      if (size <= 0)
        currnode.m_left = curr;
      else
        currnode.m_left = range.initRange.left;
      node.nodeArray[storeIdx].externalArray = currnode;
      break;
    }
  }
  if (type >= ARRAY_LEAF_NODE && firstLeaf == -1) {
    firstLeaf = storeIdx;
  }
}

#endif  // SRC_INCLUDE_CONSTRUCT_STORE_NODE_H_
