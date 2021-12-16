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
#ifndef CONSTRUCT_STORE_NODE_H_
#define CONSTRUCT_STORE_NODE_H_

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

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
template <typename InnerNodeType>
void CARMI<KeyType, ValueType, Compare, Alloc>::StoreInnerNode(
    const DataRange &range, InnerNodeType *currnode) {
  // get the number of child nodes
  int optimalChildNumber = currnode->flagNumber & 0x00FFFFFF;
  // divide the initDataset
  SubDataset subDataset(optimalChildNumber);
  NodePartition<InnerNodeType>(*currnode, range.initRange, initDataset,
                               &(subDataset.subInit));
  NodePartition<InnerNodeType>(*currnode, range.insertRange, insertQuery,
                               &(subDataset.subInsert));
  // allocate a block of empty memory for this node in the node array
  currnode->childLeft = node.AllocateNodeMemory(optimalChildNumber);

  for (int i = 0; i < optimalChildNumber; i++) {
    // store each child node
    DataRange subRange(subDataset.subInit[i], subDataset.subFind[i],
                       subDataset.subInsert[i]);
    StoreOptimalNode(subRange, currnode->childLeft + i);
  }
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
void CARMI<KeyType, ValueType, Compare, Alloc>::StoreOptimalNode(
    const DataRange &range, int storeIdx) {
  // find the optimal setting of this sub-dataset
  auto it = structMap.find(range.initRange);

  int type = it->second.cfArray.flagNumber >> 24;
  switch (type) {
    case LR_INNER_NODE: {
      // Case 1: the optimal node is the lr inner node, use the StoreInnerNode
      // function to store itself and its child nodes.
      StoreInnerNode<LRModel<KeyType, ValueType>>(range, &(it->second.lr));
      node.nodeArray[storeIdx].lr = it->second.lr;
      break;
    }
    case PLR_INNER_NODE: {
      // Case 2: the optimal node is the p. lr inner node, use the
      // StoreInnerNode function to store itself and its child nodes.
      StoreInnerNode<PLRModel<KeyType, ValueType>>(range, &(it->second.plr));
      node.nodeArray[storeIdx].plr = it->second.plr;
      break;
    }
    case HIS_INNER_NODE: {
      // Case 3: the optimal node is the his inner node, use the StoreInnerNode
      // function to store itself and its child nodes.
      StoreInnerNode<HisModel<KeyType, ValueType>>(range, &(it->second.his));
      node.nodeArray[storeIdx].his = it->second.his;
      break;
    }
    case BS_INNER_NODE: {
      // Case 4: the optimal node is the bs inner node, use the StoreInnerNode
      // function to store itself and its child nodes.
      StoreInnerNode<BSModel<KeyType, ValueType>>(range, &(it->second.bs));
      node.nodeArray[storeIdx].bs = it->second.bs;
      break;
    }
    case ARRAY_LEAF_NODE: {
      // Case 5: the optimal node is the cf array leaf node, and then we store
      // its information in the remainingNode for future processing due to the
      // prefetching mechanism
      scanLeaf.push_back(storeIdx);
      remainingNode.push_back(storeIdx);
      remainingRange.push_back(range);
      break;
    }
    case EXTERNAL_ARRAY_LEAF_NODE: {
      // Case 6: the optimal node is the external array leaf node, store it in
      // the node array
      ExternalArray<KeyType, ValueType, Compare> currnode =
          it->second.externalArray;
      int size = range.initRange.size;
      if (size <= 0)
        currnode.m_left = prefetchEnd;
      else
        currnode.m_left = range.initRange.left;
      prefetchEnd += range.initRange.size;
      node.nodeArray[storeIdx].externalArray = currnode;
      break;
    }
  }
  if (type >= ARRAY_LEAF_NODE && range.initRange.size > 0) {
    lastLeaf = storeIdx;
    if (firstLeaf == -1) {
      firstLeaf = storeIdx;
    }
  }
}

#endif  // CONSTRUCT_STORE_NODE_H_
