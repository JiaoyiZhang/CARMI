/**
 * @file candicate_plr.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <vector>

#include "../../../../params.h"
#include "../../../construct/structures.h"

#ifndef SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_CANDIDATE_PLR_H_
#define SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_CANDIDATE_PLR_H_

struct SegmentPoint {
  float cost;
  double key[7] = {-1, -1, -1, -1, -1, -1, -1};
  int idx[8] = {-1, -1, -1, -1, -1, -1, -1};
};

class CandidateCost {
 public:
  explicit CandidateCost(int s)
      : xx(s, 0), x(s, 0), px(s, 0), pp(s, 0), p(s, 0) {
    size = s;
  }

  void StoreValue(const DataVectorType &dataset,
                  const std::vector<int> &index) {
    xx[0] = 0;
    x[0] = 0;
    px[0] = 0;
    pp[0] = 0;
    p[0] = 0;
    for (int i = 1; i < index.size(); i++) {
      for (int j = index[i - 1]; j < index[i]; j++) {
        xx[j] += dataset[j].first * dataset[j].first;
        x[j] += dataset[j].first;
        px[j] += dataset[j].first * dataset[j].second;
        pp[j] += dataset[j].second * dataset[j].second;
        p[j] += dataset[j].second;
      }
      xx[i] += xx[i - 1];
      x[i] += x[i - 1];
      px[i] += px[i - 1];
      pp[i] += pp[i - 1];
      p[i] += p[i - 1];
    }
  }

  float CalculateCost(int left, int right, DataType p1, DataType p2) {
    float a = (p2.second - p1.second) / (p2.first - p1.first);
    float b = p1.second - a * p1.first;
    float res = b * b;
    res += a * a * (xx[right] - xx[left]);
    res += 2 * a * b * (x[right] - x[left]);
    res -= 2 * a * (px[right] - px[left]);
    res += pp[right] - pp[left];
    res -= 2 * b * (p[right] - p[left]);
    return res;
  }

 public:
  int size;
  std::vector<float> xx;
  std::vector<float> x;
  std::vector<float> px;
  std::vector<float> pp;
  std::vector<float> p;
};

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

#endif  // SRC_CARMI_NODES_ROOTNODE_TRAINMODEL_CANDIDATE_PLR_H_
