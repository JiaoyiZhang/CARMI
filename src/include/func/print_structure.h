/**
 * @file print_structure.h
 * @author Jiaoyi
 * @brief print the structure of CARMI
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_FUNC_PRINT_STRUCTURE_H_
#define SRC_INCLUDE_FUNC_PRINT_STRUCTURE_H_
#include <vector>

#include "../carmi.h"

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::PrintRoot(int level, std::vector<int> *levelVec,
                                          std::vector<int> *nodeVec) {
  sumDepth += level;
  std::vector<int> tree(8, 0);
  int childNum = root.childNumber;
  for (int i = 0; i < childNum; i++) {
    auto childIdx = root.childLeft + i;
    int t = (entireChild[childIdx].lr.flagNumber >> 24);
    tree[t]++;
    (*nodeVec)[t]++;
  }
  std::cout << "\tchild:";
  if (tree[LR_INNER_NODE]) std::cout << "\tlr:" << tree[LR_INNER_NODE];
  if (tree[PLR_INNER_NODE]) std::cout << "\tplr:" << tree[PLR_INNER_NODE];
  if (tree[HIS_INNER_NODE]) std::cout << "\this:" << tree[HIS_INNER_NODE];
  if (tree[BS_INNER_NODE]) std::cout << "\tbin:" << tree[BS_INNER_NODE];
  if (tree[ARRAY_LEAF_NODE]) std::cout << "\tarray:" << tree[ARRAY_LEAF_NODE];
  if (tree[EXTERNAL_ARRAY_LEAF_NODE])
    std::cout << "\tycsb:" << tree[EXTERNAL_ARRAY_LEAF_NODE];
  std::cout << std::endl;
  for (int i = 0; i < root.childNumber; i++) {
    auto childIdx = root.childLeft + i;
    NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
    if (t >= LR_INNER_NODE && t <= BS_INNER_NODE) {
      PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
    } else {
      sumDepth += level + 1;
    }
  }
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::PrintInner(int level, int idx,
                                           std::vector<int> *levelVec,
                                           std::vector<int> *nodeVec) {
  sumDepth += level;
  std::vector<int> tree(8, 0);
  for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++) {
    auto childIdx = entireChild[idx].lr.childLeft + i;
    int t = (entireChild[childIdx].lr.flagNumber >> 24);
    tree[t]++;
    (*nodeVec)[t]++;
  }
  for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++) {
    auto childIdx = entireChild[idx].lr.childLeft + i;
    NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
    if (t > LR_ROOT_NODE && t < ARRAY_LEAF_NODE) {
      PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
    } else {
      sumDepth += level + 1;
    }
  }
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::PrintStructure(int level, NodeType type,
                                               int idx,
                                               std::vector<int> *levelVec,
                                               std::vector<int> *nodeVec) {
  (*levelVec)[level]++;
  switch (type) {
    case LR_ROOT_NODE: {
      std::cout << "level " << level << ": now root is lr, idx:" << idx
                << ", number:" << root.childNumber;
      PrintRoot(level, levelVec, nodeVec);
      break;
    }
    default:
      break;
  }
  if (type >= LR_INNER_NODE && type <= BS_INNER_NODE)
    PrintInner(level, idx, levelVec, nodeVec);
}

#endif  // SRC_INCLUDE_FUNC_PRINT_STRUCTURE_H_
