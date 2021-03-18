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

#ifndef SRC_CARMI_STATICRMI_RMI_ROOT_H_
#define SRC_CARMI_STATICRMI_RMI_ROOT_H_
#include <vector>

#include "../carmi.h"
#include "./rmi_lower.h"

// RMI
CARMI::CARMI(const DataVectorType &dataset, int childNum, int kInnerID,
             int kLeafID) {
  nowDataSize = 0;
  kLeafNodeID = kLeafID;
  kInnerNodeID = kInnerID;
  InitEntireData(0, dataset.size(), false);
  InitEntireChild(dataset.size());
  int left = AllocateChildMemory(childNum);

  std::vector<DataVectorType> perSubDataset;
  DataVectorType tmp;
  for (int i = 0; i < childNum; i++) perSubDataset.push_back(tmp);
  rootType = kInnerID;

  switch (kInnerNodeID) {
    case LR_ROOT_NODE: {
      root.lrRoot = LRType(childNum);
      root.lrRoot.childLeft = left;

      root.lrRoot.model->Train(dataset, childNum);

      for (int i = 0; i < dataset.size(); i++) {
        int p = root.lrRoot.model->Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
      }

      for (int i = 0; i < childNum; i++) {
        LRModel innerLR;
        innerLR.SetChildNumber(32);
        InitLR(perSubDataset[i], &innerLR);
        entireChild[left + i].lr = innerLR;
      }
    } break;
    case PLR_ROOT_NODE: {
      root.plrRoot = PLRType(childNum);
      root.plrRoot.childLeft = left;

      root.plrRoot.model->Train(dataset, childNum);

      for (int i = 0; i < dataset.size(); i++) {
        int p = root.plrRoot.model->Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
      }

      for (int i = 0; i < childNum; i++) {
        PLRModel innerPLR;
        innerPLR.SetChildNumber(32);
        InitPLR(perSubDataset[i], &innerPLR);
        entireChild[left + i].plr = innerPLR;
      }
    } break;
    case HIS_ROOT_NODE: {
      root.hisRoot = HisType(childNum);
      root.hisRoot.childLeft = left;

      root.hisRoot.model->Train(dataset, childNum);

      for (int i = 0; i < dataset.size(); i++) {
        int p = root.hisRoot.model->Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
      }

      for (int i = 0; i < childNum; i++) {
        HisModel innerHis;
        innerHis.SetChildNumber(32);
        InitHis(perSubDataset[i], &innerHis);
        entireChild[left + i].his = innerHis;
      }
    } break;
    case BS_ROOT_NODE: {
      root.bsRoot = BSType(childNum);
      root.bsRoot.childLeft = left;

      root.bsRoot.model->Train(dataset, childNum);

      for (int i = 0; i < dataset.size(); i++) {
        int p = root.bsRoot.model->Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
      }

      for (int i = 0; i < childNum; i++) {
        BSModel innerBS;
        innerBS.SetChildNumber(32);
        InitBS(perSubDataset[i], &innerBS);
        entireChild[left + i].bs = innerBS;
      }
    } break;
  }
};

#endif  // SRC_CARMI_STATICRMI_RMI_ROOT_H_
