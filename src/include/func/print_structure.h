/**
 * @file print_structure.h
 * @author Jiaoyi
 * @brief print the structure of CARMI
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef FUNC_PRINT_STRUCTURE_H_
#define FUNC_PRINT_STRUCTURE_H_
#include <vector>

#include "../carmi.h"

template <typename KeyType, typename ValueType>
int CARMI<KeyType, ValueType>::GetNodeInfo(int idx, int *childNumber,
                                           int *childStartIndex) {
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

#endif  // FUNC_PRINT_STRUCTURE_H_
