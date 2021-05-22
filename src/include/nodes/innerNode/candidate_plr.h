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
#include <math.h>

#include <vector>

#include "../../construct/structures.h"
#include "../../params.h"

#ifndef SRC_INCLUDE_NODES_ROOTNODE_TRAINMODEL_CANDIDATE_PLR_H_
#define SRC_INCLUDE_NODES_ROOTNODE_TRAINMODEL_CANDIDATE_PLR_H_

struct SegmentPoint {
  float cost;
  double key[7] = {-1, -1, -1, -1, -1, -1, -1};
  int idx[8] = {-1, -1, -1, -1, -1, -1, -1};
};

template <typename DataVectorType, typename DataType>
class CandidateCost {
 public:
  explicit CandidateCost(int s)
      : xx(s, 0), x(s, 0), px(s, 0), pp(s, 0), p(s, 0) {
    size = s;
  }

  void StoreValue(const DataVectorType &dataset,
                  const std::vector<int> &index) {
    xx[0] = 0.0;
    x[0] = 0.0;
    px[0] = 0.0;
    pp[0] = 0.0;
    p[0] = 0.0;
    for (int i = 1; i < static_cast<int>(index.size()); i++) {
      for (int j = index[i - 1]; j < index[i]; j++) {
        xx[i] += dataset[j].first * dataset[j].first;
        x[i] += dataset[j].first;
        px[i] += dataset[j].first * dataset[j].second;
        pp[i] += dataset[j].second * dataset[j].second;
        p[i] += dataset[j].second;
      }
      xx[i] += xx[i - 1];
      x[i] += x[i - 1];
      px[i] += px[i - 1];
      pp[i] += pp[i - 1];
      p[i] += p[i - 1];
    }
  }

  float CalculateCost(int left, int right, int size, DataType p1, DataType p2) {
    double a = (p2.second - p1.second) / (p2.first - p1.first);
    double b = p1.second - a * p1.first;
    double res = b * b * size;
    res += a * a * (xx[right] - xx[left]);
    res += 2.0 * a * (b * (x[right] - x[left]) - (px[right] - px[left]));
    res += pp[right] - pp[left];
    res -= 2.0 * b * (p[right] - p[left]);
    return res;
  }

  float Diff(int n, int len, const int idx[]) {
    int opt[8];
    for (int i = 0; i < n; i++) {
      opt[i] = fabs(static_cast<float>(len) / 8.0 * i - idx[i]);
    }
    float diff = 0.0;
    for (int i = 0; i < n; i++) {
      diff += opt[i] * opt[i];
    }
    return diff;
  }

 public:
  int size;
  std::vector<double> xx;
  std::vector<double> x;
  std::vector<double> px;
  std::vector<double> pp;
  std::vector<double> p;
};

#endif  // SRC_INCLUDE_NODES_ROOTNODE_TRAINMODEL_CANDIDATE_PLR_H_
