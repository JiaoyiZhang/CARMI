/**
 * @file print_structure.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_FUNC_PRINT_STRUCTURE_H_
#define SRC_CARMI_FUNC_PRINT_STRUCTURE_H_
#include <vector>

#include "../carmi.h"

void CARMI::PrintRoot(int level, int idx, std::vector<int> *levelVec,
                      std::vector<int> *nodeVec) const {
  std::vector<int> tree(11, 0);
  for (int i = 0; i < (root.lrRoot.flagNumber & 0x00FFFFFF); i++) {
    auto childIdx = root.lrRoot.childLeft + i;
    int t = (entireChild[childIdx].lr.flagNumber >> 24);
    tree[t]++;
    (*nodeVec)[t]++;
  }
  std::cout << "\tchild:";
  if (tree[LR_INNER_NODE]) std::cout << "\tlr:" << tree[4];
  if (tree[PLR_INNER_NODE]) std::cout << "\tplr:" << tree[5];
  if (tree[HIS_INNER_NODE]) std::cout << "\this:" << tree[6];
  if (tree[BS_INNER_NODE]) std::cout << "\tbin:" << tree[7];
  if (tree[ARRAY_LEAF_NODE]) std::cout << "\tarray:" << tree[8];
  if (tree[GAPPED_ARRAY_LEAF_NODE]) std::cout << "\tga:" << tree[9];
  if (tree[EXTERNAL_ARRAY_LEAF_NODE]) std::cout << "\tycsb:" << tree[10];
  std::cout << std::endl;
  for (int i = 0; i < (root.lrRoot.flagNumber & 0x00FFFFFF); i++) {
    auto childIdx = root.lrRoot.childLeft + i;
    NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
    if (t >= LR_INNER_NODE && t <= BS_INNER_NODE)
      PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
  }
}

void CARMI::PrintInner(int level, int idx, std::vector<int> *levelVec,
                       std::vector<int> *nodeVec) const {
  std::vector<int> tree(11, 0);
  for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++) {
    auto childIdx = entireChild[idx].lr.childLeft + i;
    int t = (entireChild[childIdx].lr.flagNumber >> 24);
    tree[t]++;
    (*nodeVec)[t]++;
  }
  for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++) {
    auto childIdx = entireChild[idx].lr.childLeft + i;
    NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
    if (t > BS_ROOT_NODE && t < ARRAY_LEAF_NODE)
      PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
  }
}

void CARMI::PrintStructure(int level, NodeType type, int idx,
                           std::vector<int> *levelVec,
                           std::vector<int> *nodeVec) const {
  (*levelVec)[level]++;
  switch (type) {
    case LR_ROOT_NODE: {
      std::cout << "level " << level << ": now root is lr, idx:" << idx
                << ", number:" << (root.lrRoot.flagNumber & 0x00FFFFFF);
      PrintRoot(level, idx, levelVec, nodeVec);
      break;
    }
    case PLR_ROOT_NODE: {
      std::cout << "level " << level << ": now root is plr, idx:" << idx
                << ", number:" << (root.plrRoot.flagNumber & 0x00FFFFFF);
      PrintRoot(level, idx, levelVec, nodeVec);
      break;
    }
    case HIS_ROOT_NODE: {
      std::cout << "level " << level << ": now root is his, idx:" << idx
                << ", number:" << (root.hisRoot.flagNumber & 0x00FFFFFF);
      PrintRoot(level, idx, levelVec, nodeVec);
      break;
    }
    case BS_ROOT_NODE: {
      std::cout << "level " << level << ": now root is bin, idx:" << idx
                << ", number:" << (root.bsRoot.flagNumber & 0x00FFFFFF);
      PrintRoot(level, idx, levelVec, nodeVec);
      break;
    }
    default:
      break;
  }
  if (type >= LR_INNER_NODE && type <= BS_INNER_NODE)
    PrintInner(level, idx, levelVec, nodeVec);
}

#endif  // SRC_CARMI_FUNC_PRINT_STRUCTURE_H_
