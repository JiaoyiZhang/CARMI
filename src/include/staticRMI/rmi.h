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

#ifndef SRC_INCLUDE_STATICRMI_RMI_H_
#define SRC_INCLUDE_STATICRMI_RMI_H_
#include <vector>

#include "../carmi.h"

// RMI
template <typename KeyType, typename ValueType>
CARMI<KeyType, ValueType>::CARMI(const DataVectorType &dataset, int childNum,
                                 int kInnerID, int kLeafID) {
  nowDataSize = 0;
  kLeafNodeID = kLeafID;
  kInnerNodeID = kInnerID;
  InitEntireData(0, dataset.size(), false);
  InitEntireChild(dataset.size());

  rootType = kInnerID;
  IndexPair range(0, initDataset.size());

  switch (kInnerNodeID) {
    case LR_ROOT_NODE:
      root = InitSRMIRoot<LRType, LinearRegression, LRModel>(childNum, range);
      break;
    case PLR_ROOT_NODE:
      root = InitSRMIRoot<PLRType, PiecewiseLR, PLRModel>(childNum, range);
      break;
    case HIS_ROOT_NODE:
      root = InitSRMIRoot<HisType, HistogramModel, HisModel>(childNum, range);
      break;
    case BS_ROOT_NODE:
      root = InitSRMIRoot<BSType, BinarySearchModel, BSModel>(childNum, range);
      break;
  }
}

template <typename KeyType, typename ValueType>
template <typename ROOTTYPE, typename ROOTMODEL, typename INNERTYPE>
inline ROOTTYPE CARMI<KeyType, ValueType>::InitSRMIRoot(
    int childNum, const IndexPair &range) {
  ROOTTYPE node(childNum);
  node.childLeft = AllocateChildMemory(childNum);

  node.model.Train(initDataset, childNum);

  std::vector<IndexPair> perSize(carmi_params::kRMIInnerChild, emptyRange);
  NodePartition<ROOTMODEL>(node.model, range, initDataset, &perSize);

  for (int i = 0; i < childNum; i++) {
    INNERTYPE inner;
    inner.SetChildNumber(32);
    InitSRMILeaf<INNERTYPE>(perSize[i], &inner);
    entireChild[node.childLeft + i] = *(reinterpret_cast<BaseNode *>(&inner));
  }
  return node;
}

template <typename KeyType, typename ValueType>
template <typename TYPE>
inline void CARMI<KeyType, ValueType>::InitSRMILeaf(const IndexPair &range,
                                                    TYPE *node) {
  node->SetChildNumber(carmi_params::kRMIInnerChild);
  int childNumber = node->flagNumber & 0x00FFFFFF;
  node->childLeft = AllocateChildMemory(childNumber);
  if (range.size == 0) return;

  Train(range.left, range.size, initDataset, node);

  std::vector<IndexPair> perSize(carmi_params::kRMIInnerChild, emptyRange);
  NodePartition<TYPE>(*node, range, initDataset, &perSize);

  switch (kLeafNodeID) {
    case ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        ArrayType tmp(kThreshold);
        Init(kMaxKeyNum, perSize[i].left, perSize[i].size, initDataset, &tmp);
        entireChild[node->childLeft + i].array = tmp;
      }
      break;
    case GAPPED_ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        GappedArrayType tmp(kThreshold);
        Init(kMaxKeyNum, perSize[i].left, perSize[i].size, initDataset, &tmp);
        entireChild[node->childLeft + i].ga = tmp;
      }
      break;
  }
}

#endif  // SRC_INCLUDE_STATICRMI_RMI_H_
