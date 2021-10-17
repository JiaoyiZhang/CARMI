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
#ifndef SRC_INCLUDE_FUNC_PRINT_STRUCTURE_H_
#define SRC_INCLUDE_FUNC_PRINT_STRUCTURE_H_
#include <vector>

#include "../carmi.h"

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::PrintRoot(int level, int dataSize,
                                          std::vector<int> *levelVec,
                                          std::vector<int> *nodeVec) {
  std::vector<int> tree(8, 0);
  int childNum = root.model.length + 1;
  for (int i = 0; i < childNum; i++) {
    auto childIdx = root.childLeft + i;
    int t = (node.nodeArray[childIdx].lr.flagNumber >> 24);
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
  for (int i = 0; i < root.model.length + 1; i++) {
    auto childIdx = root.childLeft + i;
    NodeType t = NodeType(node.nodeArray[childIdx].lr.flagNumber >> 24);
    if (t >= LR_INNER_NODE && t <= BS_INNER_NODE) {
      PrintStructure(level + 1, t, dataSize, childIdx, levelVec, nodeVec);
    } else {
      if (!isPrimary) {
        int left = node.nodeArray[childIdx].cfArray.m_left;
        int nowLeafNum =
            node.nodeArray[childIdx].cfArray.flagNumber & 0x00FFFFFF;
        int dataNum = node.nodeArray[childIdx].cfArray.GetDataNum(
            left, left + nowLeafNum);
        sumDepth += (level + 1) * dataNum * 1.0 / dataSize;
      }
    }
  }
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::PrintInner(int level, int dataSize, int idx,
                                           std::vector<int> *levelVec,
                                           std::vector<int> *nodeVec) {
  std::vector<int> tree(8, 0);
  for (int i = 0; i < (node.nodeArray[idx].lr.flagNumber & 0x00FFFFFF); i++) {
    auto childIdx = node.nodeArray[idx].lr.childLeft + i;
    int t = (node.nodeArray[childIdx].lr.flagNumber >> 24);
    tree[t]++;
    (*nodeVec)[t]++;
  }
  for (int i = 0; i < (node.nodeArray[idx].lr.flagNumber & 0x00FFFFFF); i++) {
    auto childIdx = node.nodeArray[idx].lr.childLeft + i;
    NodeType t = NodeType(node.nodeArray[childIdx].lr.flagNumber >> 24);
    if (t > PLR_ROOT_NODE && t < ARRAY_LEAF_NODE) {
      PrintStructure(level + 1, t, dataSize, childIdx, levelVec, nodeVec);
    } else {
      int left = node.nodeArray[childIdx].cfArray.m_left;
      int nowLeafNum = node.nodeArray[childIdx].cfArray.flagNumber & 0x00FFFFFF;
      int dataNum =
          node.nodeArray[childIdx].cfArray.GetDataNum(left, left + nowLeafNum);
      sumDepth += (level + 1) * dataNum * 1.0 / dataSize;
    }
  }
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::PrintStructure(int level, NodeType type,
                                               int dataSize, int idx,
                                               std::vector<int> *levelVec,
                                               std::vector<int> *nodeVec) {
  (*levelVec)[level]++;
  switch (type) {
    case PLR_ROOT_NODE: {
      std::cout << "level " << level << ": now root is plr, idx:" << idx
                << ", number:" << root.model.length + 1;
      PrintRoot(level, dataSize, levelVec, nodeVec);
      break;
    }
    default:
      break;
  }
  if (type >= LR_INNER_NODE && type <= BS_INNER_NODE)
    PrintInner(level, dataSize, idx, levelVec, nodeVec);
}

#endif  // SRC_INCLUDE_FUNC_PRINT_STRUCTURE_H_
