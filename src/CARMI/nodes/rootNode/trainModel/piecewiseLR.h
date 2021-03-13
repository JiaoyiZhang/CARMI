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

float CalculateCost(int left, int right, const DataVectorType *dataset) {
  float a = ((*dataset)[right].second - (*dataset)[left].second) /
            ((*dataset)[right].first - (*dataset)[left].first);
  float b = (*dataset)[left].second - a * (*dataset)[left].first;
  float cost = 0;
  int y;
  for (int i = left; i <= right; i++) {
    if ((*dataset)[i].first != -1) {
      y = a * (*dataset)[i].first + b;
      cost += (y - (*dataset)[i].second) * (y - (*dataset)[i].second);
    }
  }
  if (cost < 0) {
    std::cout << "wrong" << std::endl;
  }
  return cost;
}

float Diff(int n, int len, const int idx[]) {
  int opt[8];
  for (int i = 0; i < n; i++) {
    opt[i] = abs(static_cast<float>(len) / 8.0 * i - idx[i]);
  }
  float diff = 0.0;
  for (int i = 0; i < n; i++) {
    diff += opt[i] * opt[i];
  }
  return diff;
}

struct SegmentPoint {
  float cost;
  double key[7] = {-1, -1, -1, -1, -1, -1, -1};
  int idx[8] = {-1, -1, -1, -1, -1, -1, -1};
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

  SegmentPoint tmp;
  tmp.cost = DBL_MAX;
  std::vector<SegmentPoint> tmpp(size, tmp);
  std::vector<std::vector<SegmentPoint>> dp(2, tmpp);

  for (int j = 1; j < size; j++) {
    dp[0][j].cost = CalculateCost(0, j, &data);
    dp[0][j].key[0] = data[j].first;
    dp[0][j].idx[0] = data[j].second;
  }
  for (int i = 1; i < 7; i++) {
    for (int j = i + 1; j < size - 1; j++) {
      SegmentPoint opt;
      opt.cost = DBL_MAX;
      for (int k = 1; k < j; k++) {
        float res = DBL_MAX;
        if (i < 6) {
          res = dp[0][k].cost + CalculateCost(k, j, &data);
        } else {
          res = dp[0][k].cost + CalculateCost(k, j, &data) +
                CalculateCost(j, size - 1, &data);
        }
        if (res < opt.cost) {
          opt.cost = res;
          for (int l = 0; l < i; l++) {
            opt.idx[l] = dp[0][k].idx[l];
            opt.key[l] = dp[0][k].key[l];
          }
          opt.key[i] = data[j].first;
          opt.idx[i] = data[j].second;
        } else if (res == opt.cost) {
          int dp_idx[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
          for (int l = 0; l < i; l++) {
            dp_idx[l] = dp[0][k].idx[l];
          }
          dp_idx[i] = data[j].second;

          float var0 = Diff(i + 1, len, dp_idx);
          float var1 = Diff(i + 1, len, opt.idx);
          if (var0 < var1) {
            for (int l = 0; l < i; l++) {
              opt.idx[l] = dp[0][k].idx[l];
              opt.key[l] = dp[0][k].key[l];
            }
            opt.key[i] = data[j].first;
            opt.idx[i] = data[j].second;
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
  for (int j = 7; j < size; j++) {
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
