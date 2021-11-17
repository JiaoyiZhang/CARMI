/**
 * @file get_node_info.h
 * @author Jiaoyi
 * @brief get the information of the node in CARMI
 * @version 3.0
 * @date 2021-10-24
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef FUNC_GET_NODE_INFO_H_
#define FUNC_GET_NODE_INFO_H_
#include <vector>

#include "../carmi.h"

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
int CARMI<KeyType, ValueType, Compare, Alloc>::GetNodeInfo(
    int idx, int *childNumber, int *childStartIndex) {
  // Case 1: the index of the node is invalid
  if (idx < 0 || idx >= node.nowNodeNumber) {
    return -1;
  }
  // Case 2: the node is valid
  int type = node.nodeArray[idx].lr.flagNumber >> 24;
  *childNumber = node.nodeArray[idx].lr.flagNumber & 0xFFFFFF;
  *childStartIndex = node.nodeArray[idx].lr.childLeft;
  return type;
}

#endif  // FUNC_GET_NODE_INFO_H_
