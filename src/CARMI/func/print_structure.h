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

void CARMI::PrintStructure(int level, NodeType type, int idx,
                           std::vector<int> *levelVec,
                           std::vector<int> *nodeVec) const {
  (*levelVec)[level]++;
  std::vector<int> tree;
  for (int i = 0; i < 11; i++) tree.push_back(0);
  switch (type) {
    case LR_ROOT_NODE: {
      std::cout << "level " << level << ": now root is lr, idx:" << idx
                << ", number:" << (root.lrRoot.flagNumber & 0x00FFFFFF);
      for (int i = 0; i < (root.lrRoot.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = root.lrRoot.childLeft + i;
        int t = (entireChild[childIdx].lr.flagNumber >> 24);
        tree[t]++;
        (*nodeVec)[t]++;
      }
      std::cout << "\tchild:";
      if (tree[4]) std::cout << "\tlr:" << tree[4];
      if (tree[5]) std::cout << "\tplr:" << tree[5];
      if (tree[6]) std::cout << "\this:" << tree[6];
      if (tree[7]) std::cout << "\tbin:" << tree[7];
      if (tree[8]) std::cout << "\tarray:" << tree[8];
      if (tree[9]) std::cout << "\tga:" << tree[9];
      if (tree[10]) std::cout << "\tycsb:" << tree[10];
      std::cout << std::endl;
      for (int i = 0; i < (root.lrRoot.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = root.lrRoot.childLeft + i;
        NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
        if (t > BS_ROOT_NODE && t < ARRAY_LEAF_NODE)
          PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
      }
      break;
    }
    case PLR_ROOT_NODE: {
      std::cout << "level " << level << ": now root is plr, idx:" << idx
                << ", number:" << (root.plrRoot.flagNumber & 0x00FFFFFF);
      for (int i = 0; i < (root.plrRoot.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = root.plrRoot.childLeft + i;
        int t = (entireChild[childIdx].plr.flagNumber >> 24);
        tree[t]++;
        (*nodeVec)[t]++;
      }
      std::cout << "\tchild:";
      if (tree[4]) std::cout << "\tlr:" << tree[4];
      if (tree[5]) std::cout << "\tplr:" << tree[5];
      if (tree[6]) std::cout << "\this:" << tree[6];
      if (tree[7]) std::cout << "\tbin:" << tree[7];
      if (tree[8]) std::cout << "\tarray:" << tree[8];
      if (tree[9]) std::cout << "\tga:" << tree[9];
      if (tree[10]) std::cout << "\tycsb:" << tree[10];
      std::cout << std::endl;
      for (int i = 0; i < (root.plrRoot.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = root.plrRoot.childLeft + i;
        NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
        if (t > BS_ROOT_NODE && t < ARRAY_LEAF_NODE)
          PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
      }
      break;
    }
    case HIS_ROOT_NODE: {
      std::cout << "level " << level << ": now root is his, idx:" << idx
                << ", number:" << (root.hisRoot.flagNumber & 0x00FFFFFF);
      for (int i = 0; i < (root.hisRoot.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = root.hisRoot.childLeft + i;
        int t = (entireChild[childIdx].his.flagNumber >> 24);
        tree[t]++;
        (*nodeVec)[t]++;
      }
      std::cout << "\tchild:";
      if (tree[4]) std::cout << "\tlr:" << tree[4];
      if (tree[5]) std::cout << "\tplr:" << tree[5];
      if (tree[6]) std::cout << "\this:" << tree[6];
      if (tree[7]) std::cout << "\tbin:" << tree[7];
      if (tree[8]) std::cout << "\tarray:" << tree[8];
      if (tree[9]) std::cout << "\tga:" << tree[9];
      if (tree[10]) std::cout << "\tycsb:" << tree[10];
      std::cout << std::endl;
      for (int i = 0; i < (root.hisRoot.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = root.hisRoot.childLeft + i;
        NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
        if (t > BS_ROOT_NODE && t < ARRAY_LEAF_NODE)
          PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
      }
      break;
    }
    case BS_ROOT_NODE: {
      std::cout << "level " << level << ": now root is bin, idx:" << idx
                << ", number:" << (root.bsRoot.flagNumber & 0x00FFFFFF);
      for (int i = 0; i < (root.bsRoot.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = root.bsRoot.childLeft + i;
        int t = (entireChild[childIdx].bs.flagNumber >> 24);
        tree[t]++;
        (*nodeVec)[t]++;
      }
      std::cout << "\tchild:";
      if (tree[4]) std::cout << "\tlr:" << tree[4];
      if (tree[5]) std::cout << "\tplr:" << tree[5];
      if (tree[6]) std::cout << "\this:" << tree[6];
      if (tree[7]) std::cout << "\tbin:" << tree[7];
      if (tree[8]) std::cout << "\tarray:" << tree[8];
      if (tree[9]) std::cout << "\tga:" << tree[9];
      if (tree[10]) std::cout << "\tycsb:" << tree[10];
      std::cout << std::endl;
      for (int i = 0; i < (root.bsRoot.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = root.bsRoot.childLeft + i;
        NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
        if (t > BS_ROOT_NODE && t < ARRAY_LEAF_NODE)
          PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
      }
      break;
    }

    case LR_INNER_NODE: {
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
      break;
    }
    case PLR_INNER_NODE: {
      for (int i = 0; i < (entireChild[idx].plr.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = entireChild[idx].plr.childLeft + i;
        int t = (entireChild[childIdx].plr.flagNumber >> 24);
        tree[t]++;
        (*nodeVec)[t]++;
      }
      for (int i = 0; i < (entireChild[idx].plr.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = entireChild[idx].plr.childLeft + i;
        NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
        if (t > BS_ROOT_NODE && t < ARRAY_LEAF_NODE)
          PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
      }
      break;
    }
    case HIS_INNER_NODE: {
      for (int i = 0; i < (entireChild[idx].his.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = entireChild[idx].plr.childLeft + i;
        int t = (entireChild[childIdx].his.flagNumber >> 24);
        tree[t]++;
        (*nodeVec)[t]++;
      }
      for (int i = 0; i < (entireChild[idx].his.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = entireChild[idx].his.childLeft + i;
        NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
        if (t > BS_ROOT_NODE && t < ARRAY_LEAF_NODE)
          PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
      }
      break;
    }
    case BS_INNER_NODE: {
      for (int i = 0; i < (entireChild[idx].bs.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = entireChild[idx].bs.childLeft + i;
        int t = (entireChild[childIdx].bs.flagNumber >> 24);
        tree[t]++;
        (*nodeVec)[t]++;
      }
      for (int i = 0; i < (entireChild[idx].bs.flagNumber & 0x00FFFFFF); i++) {
        auto childIdx = entireChild[idx].bs.childLeft + i;
        NodeType t = NodeType(entireChild[childIdx].lr.flagNumber >> 24);
        if (t > BS_ROOT_NODE && t < ARRAY_LEAF_NODE)
          PrintStructure(level + 1, t, childIdx, levelVec, nodeVec);
      }
      break;
    }
    default:
      break;
  }
}

#endif  // SRC_CARMI_FUNC_PRINT_STRUCTURE_H_
