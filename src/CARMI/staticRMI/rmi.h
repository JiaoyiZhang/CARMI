/**
 * @file static_rmi.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-18
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SRC_CARMI_STATICRMI_RMI_H_
#define SRC_CARMI_STATICRMI_RMI_H_
#include <vector>

#include "../carmi.h"

// RMI
CARMI::CARMI(const DataVectorType &dataset, int childNum, int kInnerID,
             int kLeafID) {
  nowDataSize = 0;
  kLeafNodeID = kLeafID;
  kInnerNodeID = kInnerID;
  InitEntireData(0, dataset.size(), false);
  InitEntireChild(dataset.size());

  rootType = kInnerID;
  IndexPair range(0, initDataset.size());

  switch (kInnerNodeID) {
    case LR_ROOT_NODE:
      root.lrRoot = InitSRMIRoot<LRType, LRModel>(childNum, range);
      break;
    case PLR_ROOT_NODE:
      root.plrRoot = InitSRMIRoot<PLRType, PLRModel>(childNum, range);
      break;
    case HIS_ROOT_NODE:
      root.hisRoot = InitSRMIRoot<HisType, HisModel>(childNum, range);
      break;
    case BS_ROOT_NODE:
      root.bsRoot = InitSRMIRoot<BSType, BSModel>(childNum, range);
      break;
  }
}

template <typename ROOTTYPE, typename INNERTYPE>
inline ROOTTYPE CARMI::InitSRMIRoot(int childNum, const IndexPair &range) {
  ROOTTYPE node(childNum);
  node.childLeft = AllocateChildMemory(childNum);

  root.bsRoot.model->Train(initDataset, childNum);

  std::vector<IndexPair> perSize(kRMIInnerChild, emptyRange);
  NodePartition<ROOTTYPE>(node, range, initDataset, &perSize);

  for (int i = 0; i < childNum; i++) {
    INNERTYPE inner;
    inner.SetChildNumber(32);
    InitSRMILeaf<INNERTYPE>(perSize[i], &inner);
    entireChild[node.childLeft + i] = *(reinterpret_cast<BaseNode *>(&inner));
  }
  return node;
}

template <typename TYPE>
inline void CARMI::InitSRMILeaf(const IndexPair &range, TYPE *node) {
  node->SetChildNumber(kRMIInnerChild);
  int childNumber = node->flagNumber & 0x00FFFFFF;
  node->childLeft = AllocateChildMemory(childNumber);
  if (range.size == 0) return;

  Train(range.left, range.size, initDataset, node);

  std::vector<IndexPair> perSize(kRMIInnerChild, emptyRange);
  NodePartition<TYPE>(*node, range, initDataset, &perSize);

  switch (kLeafNodeID) {
    case ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        ArrayType tmp(kThreshold);
        InitArray(kMaxKeyNum, perSize[i].left, perSize[i].size, initDataset,
                  &tmp);
        entireChild[node->childLeft + i].array = tmp;
      }
      break;
    case GAPPED_ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        GappedArrayType tmp(kThreshold);
        InitGA(kMaxKeyNum, perSize[i].left, perSize[i].size, initDataset, &tmp);
        entireChild[node->childLeft + i].ga = tmp;
      }
      break;
  }
}

#endif  // SRC_CARMI_STATICRMI_RMI_H_
