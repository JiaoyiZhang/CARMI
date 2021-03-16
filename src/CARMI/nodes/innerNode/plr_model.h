/**
 * @file plr_model.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_PLR_MODEL_H_
#define SRC_CARMI_NODES_INNERNODE_PLR_MODEL_H_

#include <float.h>

#include <utility>
#include <vector>

#include "../../carmi.h"
#include "../rootNode/trainModel/candidate_plr.h"

inline void CARMI::initPLR(const DataVectorType &dataset, PLRModel *plr) {
  int childNumber = plr->flagNumber & 0x00FFFFFF;
  plr->childLeft = allocateChildMemory(childNumber);
  if (dataset.size() == 0) return;

  Train(0, dataset.size(), dataset, plr);

  std::vector<DataVectorType> perSubDataset;
  DataVectorType tmp;
  for (int i = 0; i < childNumber; i++) perSubDataset.push_back(tmp);
  for (int i = 0; i < dataset.size(); i++) {
    int p = plr->Predict(dataset[i].first);
    perSubDataset[p].push_back(dataset[i]);
  }

  switch (kLeafNodeID) {
    case ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        ArrayType tmp(kThreshold);
        initArray(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i],
                  &tmp);
        entireChild[plr->childLeft + i].array = tmp;
      }
      break;
    case GAPPED_ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        GappedArrayType tmp(kThreshold);
        initGA(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i], &tmp);
        entireChild[plr->childLeft + i].ga = tmp;
      }
      break;
  }
}

inline void CARMI::Train(const int left, const int size,
                         const DataVectorType &dataset, PLRModel *plr) {
  int childNumber = plr->flagNumber & 0x00FFFFFF;
  DataVectorType data(size, {-1, 0});

  int end = left + size;
  plr->keys[0] = dataset[left].first;
  plr->keys[7] = dataset[end].first;
  for (int i = left, j = 0; i < end; i++, j++) {
    data[j].first = dataset[i].first;
    data[j].second =
        static_cast<int>(static_cast<float>(j) / data.size() * childNumber - 1);
  }

  int cand_size = 100;
  DataVectorType cand_point(cand_size, {0, 0});
  std::vector<int> cand_index(cand_size, 0);
  CandidateCost cand_cost(cand_size);
  int seg = size / cand_size;
  for (int i = 0; i < cand_size - 1; i++) {
    cand_index[i] = i * seg;
    cand_point[i] = data[i * seg];
  }
  cand_index[cand_size - 1] = size - 1;
  cand_point[cand_size - 1] = data[size - 1];
  cand_cost.StoreValue(data, cand_index);

  SegmentPoint tmp;
  tmp.cost = DBL_MAX;
  std::vector<SegmentPoint> tmpp(cand_size, tmp);
  std::vector<std::vector<SegmentPoint>> dp(2, tmpp);

  for (int j = 1; j < cand_size; j++) {
    dp[0][j].cost = cand_cost.CalculateCost(0, cand_index[j], cand_point[0],
                                            cand_point[cand_index[j]]);
    dp[0][j].key[0] = cand_point[cand_index[j]].first;
    dp[0][j].idx[0] = cand_point[cand_index[j]].second;
  }

  for (int i = 1; i < 6; i++) {
    for (int j = i + 1; j < cand_size - 1; j++) {
      SegmentPoint opt;
      opt.cost = DBL_MAX;
      for (int k = 1; k < j; k++) {
        float res = DBL_MAX;
        if (i < 5) {
          res = dp[0][k].cost +
                cand_cost.CalculateCost(cand_index[k], cand_index[j],
                                        cand_point[cand_index[k]],
                                        cand_point[cand_index[j]]);
        } else {
          res =
              dp[0][k].cost +
              cand_cost.CalculateCost(cand_index[k], cand_index[j],
                                      cand_point[cand_index[k]],
                                      cand_point[cand_index[j]]) +
              cand_cost.CalculateCost(cand_index[j], cand_index[cand_size - 1],
                                      cand_point[cand_index[j]],
                                      cand_point[cand_index[cand_size - 1]]);
        }
        if (res < opt.cost) {
          opt.cost = res;
          for (int l = 0; l < i; l++) {
            opt.idx[l] = dp[0][k].idx[l];
            opt.key[l] = dp[0][k].key[l];
          }
          opt.key[i] = cand_point[cand_index[j]].first;
          opt.idx[i] = cand_point[cand_index[j]].second;
        } else if (res == opt.cost) {
          int dp_idx[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
          for (int l = 0; l < i; l++) {
            dp_idx[l] = dp[0][k].idx[l];
          }
          dp_idx[i] = cand_point[cand_index[j]].second;

          float var0 = Diff(i + 1, childNumber, dp_idx);
          float var1 = Diff(i + 1, childNumber, opt.idx);
          if (var0 < var1) {
            for (int l = 0; l < i; l++) {
              opt.idx[l] = dp[0][k].idx[l];
              opt.key[l] = dp[0][k].key[l];
            }
            opt.key[i] = cand_point[cand_index[j]].first;
            opt.idx[i] = cand_point[cand_index[j]].second;
          }
        }
      }
      dp[1][j].cost = opt.cost;
      for (int l = 0; l <= i; l++) {
        dp[1][j].idx[l] = opt.idx[l];
        dp[1][j].key[l] = opt.key[l];
      }
    }
    for (int m = i + 1; m < size - 1; m++) {
      dp[0][m].cost = dp[1][m].cost;
      for (int l = 0; l <= i; l++) {
        dp[0][m].idx[l] = dp[1][m].idx[l];
        dp[0][m].key[l] = dp[1][m].key[l];
      }
    }
  }

  SegmentPoint opt;
  opt.cost = DBL_MAX;
  for (int j = 6; j < size; j++) {
    if (dp[1][j].cost < opt.cost) {
      opt.cost = dp[1][j].cost;
      for (int k = 0; k < 6; k++) {
        opt.idx[k] = dp[1][j].idx[k];
        opt.key[k] = dp[1][j].key[k];
      }
    } else if (dp[1][j].cost == opt.cost) {
      dp[1][j].idx[6] = childNumber - 1;
      opt.idx[6] = childNumber - 1;
      float var0 = Diff(7, childNumber, dp[1][j].idx);
      float var1 = Diff(7, childNumber, opt.idx);
      if (var0 < var1) {
        opt.cost = dp[1][j].cost;
        for (int k = 0; k < 6; k++) {
          opt.idx[k] = dp[1][j].idx[k];
          opt.key[k] = dp[1][j].key[k];
        }
      }
    }
  }
  for (int i = 0; i < 6; i++) {
    plr->keys[i + 1] = opt.key[i];
    plr->index[i] = opt.idx[i];
  }
}

inline void CARMI::Train(const int left, const int size, PLRModel *plr) {
  int childNumber = plr->flagNumber & 0x00FFFFFF;
  DataVectorType data(size, {-1, 0});

  int end = left + size;
  plr->keys[0] = initDataset[left].first;
  plr->keys[7] = initDataset[end].first;
  for (int i = left, j = 0; i < end; i++, j++) {
    data[j].first = initDataset[i].first;
    data[j].second =
        static_cast<int>(static_cast<float>(j) / data.size() * childNumber - 1);
  }

  int cand_size = 100;
  DataVectorType cand_point(cand_size, {0, 0});
  std::vector<int> cand_index(cand_size, 0);
  CandidateCost cand_cost(cand_size);
  int seg = size / cand_size;
  for (int i = 0; i < cand_size - 1; i++) {
    cand_index[i] = i * seg;
    cand_point[i] = data[i * seg];
  }
  cand_index[cand_size - 1] = size - 1;
  cand_point[cand_size - 1] = data[size - 1];
  cand_cost.StoreValue(data, cand_index);

  SegmentPoint tmp;
  tmp.cost = DBL_MAX;
  std::vector<SegmentPoint> tmpp(cand_size, tmp);
  std::vector<std::vector<SegmentPoint>> dp(2, tmpp);

  for (int j = 1; j < cand_size; j++) {
    dp[0][j].cost = cand_cost.CalculateCost(0, cand_index[j], cand_point[0],
                                            cand_point[cand_index[j]]);
    dp[0][j].key[0] = cand_point[cand_index[j]].first;
    dp[0][j].idx[0] = cand_point[cand_index[j]].second;
  }

  for (int i = 1; i < 6; i++) {
    for (int j = i + 1; j < cand_size - 1; j++) {
      SegmentPoint opt;
      opt.cost = DBL_MAX;
      for (int k = 1; k < j; k++) {
        float res = DBL_MAX;
        if (i < 5) {
          res = dp[0][k].cost +
                cand_cost.CalculateCost(cand_index[k], cand_index[j],
                                        cand_point[cand_index[k]],
                                        cand_point[cand_index[j]]);
        } else {
          res =
              dp[0][k].cost +
              cand_cost.CalculateCost(cand_index[k], cand_index[j],
                                      cand_point[cand_index[k]],
                                      cand_point[cand_index[j]]) +
              cand_cost.CalculateCost(cand_index[j], cand_index[cand_size - 1],
                                      cand_point[cand_index[j]],
                                      cand_point[cand_index[cand_size - 1]]);
        }
        if (res < opt.cost) {
          opt.cost = res;
          for (int l = 0; l < i; l++) {
            opt.idx[l] = dp[0][k].idx[l];
            opt.key[l] = dp[0][k].key[l];
          }
          opt.key[i] = cand_point[cand_index[j]].first;
          opt.idx[i] = cand_point[cand_index[j]].second;
        } else if (res == opt.cost) {
          int dp_idx[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
          for (int l = 0; l < i; l++) {
            dp_idx[l] = dp[0][k].idx[l];
          }
          dp_idx[i] = cand_point[cand_index[j]].second;

          float var0 = Diff(i + 1, childNumber, dp_idx);
          float var1 = Diff(i + 1, childNumber, opt.idx);
          if (var0 < var1) {
            for (int l = 0; l < i; l++) {
              opt.idx[l] = dp[0][k].idx[l];
              opt.key[l] = dp[0][k].key[l];
            }
            opt.key[i] = cand_point[cand_index[j]].first;
            opt.idx[i] = cand_point[cand_index[j]].second;
          }
        }
      }
      dp[1][j].cost = opt.cost;
      for (int l = 0; l <= i; l++) {
        dp[1][j].idx[l] = opt.idx[l];
        dp[1][j].key[l] = opt.key[l];
      }
    }
    for (int m = i + 1; m < size - 1; m++) {
      dp[0][m].cost = dp[1][m].cost;
      for (int l = 0; l <= i; l++) {
        dp[0][m].idx[l] = dp[1][m].idx[l];
        dp[0][m].key[l] = dp[1][m].key[l];
      }
    }
  }

  SegmentPoint opt;
  opt.cost = DBL_MAX;
  for (int j = 6; j < size; j++) {
    if (dp[1][j].cost < opt.cost) {
      opt.cost = dp[1][j].cost;
      for (int k = 0; k < 6; k++) {
        opt.idx[k] = dp[1][j].idx[k];
        opt.key[k] = dp[1][j].key[k];
      }
    } else if (dp[1][j].cost == opt.cost) {
      dp[1][j].idx[6] = childNumber - 1;
      opt.idx[6] = childNumber - 1;
      float var0 = Diff(7, childNumber, dp[1][j].idx);
      float var1 = Diff(7, childNumber, opt.idx);
      if (var0 < var1) {
        opt.cost = dp[1][j].cost;
        for (int k = 0; k < 6; k++) {
          opt.idx[k] = dp[1][j].idx[k];
          opt.key[k] = dp[1][j].key[k];
        }
      }
    }
  }

  for (int i = 0; i < 6; i++) {
    plr->keys[i + 1] = opt.key[i];
    plr->index[i] = opt.idx[i];
  }
}

inline int PLRModel::Predict(double key) const {
  int s = 0;
  int e = 7;
  int mid;
  while (s < e) {
    mid = (s + e) / 2;
    if (keys[mid] < key)
      s = mid + 1;
    else
      e = mid;
  }

  if (e == 0) {
    return 0;
  }
  int p;
  int childNumber = flagNumber & 0x00FFFFFF;
  if (e == 5) {
    float a =
        static_cast<float>(childNumber - 1 - index[5]) / (keys[7] - keys[6]);
    float b = childNumber - 1 - a * keys[7];
    p = a * key + b;
  } else {
    float a =
        static_cast<float>(index[e + 1] - index[e]) / (keys[e] - keys[e - 1]);
    float b = index[e + 1] - a * keys[e];
    p = a * key + b;
  }
  if (p >= childNumber) p = childNumber - 1;

  return p;
}

#endif  // SRC_CARMI_NODES_INNERNODE_PLR_MODEL_H_
