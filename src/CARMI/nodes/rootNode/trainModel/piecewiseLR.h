/**
 * @file piecewiseLR.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_PIECEWISELR_H_
#define SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_PIECEWISELR_H_

#include <float.h>
#include <math.h>

#include <algorithm>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "../../../../params.h"
#include "../../../construct/structures.h"
#include "./candidate_plr.h"

class PiecewiseLR {
 public:
  PiecewiseLR() {
    for (int i = 0; i < 9; i++) {
      point.push_back({-1, 0});
    }
  }
  PiecewiseLR(const PiecewiseLR &node) {
    length = node.length;
    point = node.point;
  }

  void Train(const DataVectorType &dataset, int len);
  int Predict(double key) const {
    int s = 0;
    int e = 8;
    int mid;
    while (s < e) {
      mid = (s + e) / 2;
      if (point[mid].first < key)
        s = mid + 1;
      else
        e = mid;
    }
    if (e == 0) {
      return 0;
    }
    // return the predicted idx in the children
    float a = (point[e].second - point[e - 1].second) /
              (point[e].first - point[e - 1].first);
    float b = point[e].second - a * point[e].first;
    int p = a * key + b;
    if (p < point[e - 1].second)
      p = point[e - 1].second;
    else if (p > point[e].second)
      p = point[e].second;
    return p;
  }

 private:
  int length;
  std::vector<std::pair<float, int>> point;  // <point.first, boundary>
};

void PiecewiseLR::Train(const DataVectorType &dataset, int len) {
  DataVectorType data;
  data.insert(data.begin(), dataset.begin(), dataset.end());
  length = len - 1;
  for (int i = 0; i < 9; i++) {
    point[i] = {data[data.size() - 1].first, length};
  }
  point[0].first = dataset[0].first;
  point[0].second = 0;

  int actualSize = 0;
  for (int i = 0; i < data.size(); i++) {
    if (data[i].first != -1) {
      actualSize++;
    }
    data[i].second =
        static_cast<int>(static_cast<float>(i) / data.size() * len);
  }
  if (actualSize == 0) return;
  int size = data.size();
  int cand_size = 1000;
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
    dp[0][j].cost = cand_cost.CalculateCost(0, j, cand_index[j], cand_point[0],
                                            cand_point[j]);
    dp[0][j].key[0] = cand_point[j].first;
    dp[0][j].idx[0] = cand_point[j].second;
  }

  for (int i = 1; i < 7; i++) {
    for (int j = i + 1; j < cand_size - 1; j++) {
      SegmentPoint opt;
      opt.cost = DBL_MAX;
      for (int k = 1; k < j; k++) {
        float res = DBL_MAX;
        if (i < 6)
          res = dp[0][k].cost +
                cand_cost.CalculateCost(k, j, cand_index[j] - cand_index[k],
                                        cand_point[k], cand_point[j]);
        else
          res = dp[0][k].cost +
                cand_cost.CalculateCost(k, j, cand_index[j] - cand_index[k],
                                        cand_point[k], cand_point[j]) +
                cand_cost.CalculateCost(
                    j, cand_size - 1, cand_index[cand_size - 1] - cand_index[j],
                    cand_point[j], cand_point[cand_size - 1]);

        if (res < opt.cost) {
          opt.cost = res;
          for (int l = 0; l < i; l++) {
            opt.idx[l] = dp[0][k].idx[l];
            opt.key[l] = dp[0][k].key[l];
          }
          opt.key[i] = cand_point[j].first;
          opt.idx[i] = cand_point[j].second;
        } else if (res == opt.cost) {
          int dp_idx[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
          for (int l = 0; l < i; l++) {
            dp_idx[l] = dp[0][k].idx[l];
          }
          dp_idx[i] = cand_point[j].second;

          float var0 = Diff(i + 1, len, dp_idx);
          float var1 = Diff(i + 1, len, opt.idx);
          if (var0 < var1) {
            for (int l = 0; l < i; l++) {
              opt.idx[l] = dp[0][k].idx[l];
              opt.key[l] = dp[0][k].key[l];
            }
            opt.key[i] = cand_point[j].first;
            opt.idx[i] = cand_point[j].second;
          }
        }
      }
      dp[1][j].cost = opt.cost;
      for (int l = 0; l <= i; l++) {
        dp[1][j].idx[l] = opt.idx[l];
        dp[1][j].key[l] = opt.key[l];
      }
    }
    for (int m = i + 1; m < cand_size - 1; m++) {
      dp[0][m].cost = dp[1][m].cost;
      for (int l = 0; l <= i; l++) {
        dp[0][m].idx[l] = dp[1][m].idx[l];
        dp[0][m].key[l] = dp[1][m].key[l];
      }
    }
  }

  SegmentPoint opt;
  opt.cost = DBL_MAX;
  for (int j = 7; j < cand_size; j++) {
    if (dp[1][j].cost < opt.cost) {
      opt.cost = dp[1][j].cost;
      for (int k = 0; k < 7; k++) {
        opt.idx[k] = dp[1][j].idx[k];
        opt.key[k] = dp[1][j].key[k];
      }
    } else if (dp[1][j].cost == opt.cost) {
      dp[1][j].idx[7] = length;
      opt.idx[7] = length;
      float var0 = Diff(8, len, dp[1][j].idx);
      float var1 = Diff(8, len, opt.idx);
      if (var0 < var1) {
        opt.cost = dp[1][j].cost;
        for (int k = 0; k < 7; k++) {
          opt.idx[k] = dp[1][j].idx[k];
          opt.key[k] = dp[1][j].key[k];
        }
      }
    }
  }
  for (int i = 0; i < 7; i++) {
    point[i + 1].first = opt.key[i];
    point[i + 1].second = opt.idx[i];
  }
}
#endif  // SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_PIECEWISELR_H_
