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
#ifndef SRC_INCLUDE_NODES_INNERNODE_PLR_MODEL_H_
#define SRC_INCLUDE_NODES_INNERNODE_PLR_MODEL_H_

#include <float.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "../../carmi.h"
#include "./candidate_plr.h"

template <typename KeyType, typename ValueType>
inline void CARMI<KeyType, ValueType>::Train(const int left, const int size,
                                             const DataVectorType &dataset,
                                             PLRModel *plr) {
  int childNumber = plr->flagNumber & 0x00FFFFFF;
  DataVectorType data(size, {-1, 0});

  int end = left + size;
  plr->keys[0] = dataset[left].first;
  plr->keys[7] = dataset[end - 1].first;
  double avg = 0.0;
  for (int i = left, j = 0; i < end; i++, j++) {
    avg += dataset[i].first / size;
    data[j].second = static_cast<float>(j) / size * (childNumber - 1);
  }

  // normalize
  for (int i = left, j = 0; i < end; i++, j++) {
    data[j].first = dataset[i].first - avg;
  }

  int cand_size = std::min(size, 100);
  DataVectorType cand_point(cand_size, {0, 0});
  std::vector<int> cand_index(cand_size, 0);
  CandidateCost<DataVectorType, DataType> cand_cost(cand_size);
  float seg = size * 1.0 / cand_size;
  for (int i = 0; i < cand_size - 1; i++) {
    if (i * seg >= size) {
      for (; i < cand_size - 1; i++) {
        cand_index[i] = size - 1;
        cand_point[i] = data[size - 1];
      }
      break;
    }
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
    dp[0][j].key[0] = cand_point[0].first;
    dp[0][j].idx[0] = cand_point[0].second;
    dp[0][j].key[1] = cand_point[j].first;
    dp[0][j].idx[1] = cand_point[j].second;
    dp[0][j].key[7] = cand_point[cand_size - 1].first;
    dp[0][j].idx[7] = cand_point[cand_size - 1].second;
  }

  for (int i = 1; i < 6; i++) {
    for (int j = i + 1; j < cand_size - 1; j++) {
      SegmentPoint opt;
      opt.cost = DBL_MAX;
      for (int k = i; k < j; k++) {
        float res = DBL_MAX;
        if (i < 5) {
          res = dp[0][k].cost +
                cand_cost.CalculateCost(k, j, cand_index[j] - cand_index[k],
                                        cand_point[k], cand_point[j]);
        } else {
          res = dp[0][k].cost +
                cand_cost.CalculateCost(k, j, cand_index[j] - cand_index[k],
                                        cand_point[k], cand_point[j]) +
                cand_cost.CalculateCost(
                    j, cand_size - 1, cand_index[cand_size - 1] - cand_index[j],
                    cand_point[j], cand_point[cand_size - 1]);
        }
        if (res < opt.cost) {
          opt.cost = res;
          for (int l = 0; l <= i; l++) {
            opt.idx[l] = dp[0][k].idx[l];
            opt.key[l] = dp[0][k].key[l];
          }
          opt.key[i + 1] = cand_point[j].first;
          opt.idx[i + 1] = cand_point[j].second;
        } else if (res == opt.cost) {
          float dp_idx[8] = {0,  -1, -1, -1,
                             -1, -1, -1, static_cast<float>(childNumber - 1)};
          for (int l = 0; l <= i; l++) {
            dp_idx[l + 1] = dp[0][k].idx[l];
          }
          dp_idx[i] = cand_point[j].second;
          int tmpLen = i + 2;
          if (tmpLen == 7) {
            tmpLen = 8;
          }

          float var0 = cand_cost.Diff(tmpLen, childNumber, dp_idx);
          float var1 = cand_cost.Diff(tmpLen, childNumber, opt.idx);
          if (var0 < var1) {
            for (int l = 0; l <= i; l++) {
              opt.idx[l] = dp[0][k].idx[l];
              opt.key[l] = dp[0][k].key[l];
            }
            opt.key[i + 1] = cand_point[j].first;
            opt.idx[i + 1] = cand_point[j].second;
          }
        }
      }
      dp[1][j].cost = opt.cost;
      for (int l = 0; l <= i + 1; l++) {
        dp[1][j].idx[l] = opt.idx[l];
        dp[1][j].key[l] = opt.key[l];
      }
    }
    if (i != 5) {
      for (int m = i + 1; m < cand_size - 1; m++) {
        dp[0][m].cost = dp[1][m].cost;
        for (int l = 0; l <= i + 1; l++) {
          dp[0][m].idx[l] = dp[1][m].idx[l];
          dp[0][m].key[l] = dp[1][m].key[l];
        }
      }
    }
  }

  SegmentPoint opt;
  opt.cost = DBL_MAX;
  opt.idx[7] = cand_point[cand_size - 1].second;
  opt.key[7] = cand_point[cand_size - 1].first;
  for (int j = 6; j < cand_size; j++) {
    if (dp[1][j].cost < opt.cost) {
      opt.cost = dp[1][j].cost;
      for (int k = 0; k <= 6; k++) {
        opt.idx[k] = dp[1][j].idx[k];
        opt.key[k] = dp[1][j].key[k];
      }
    } else if (dp[1][j].cost == opt.cost) {
      dp[1][j].idx[7] = childNumber - 1;
      float var0 = cand_cost.Diff(8, childNumber, dp[1][j].idx);
      float var1 = cand_cost.Diff(8, childNumber, opt.idx);
      if (var0 < var1) {
        opt.cost = dp[1][j].cost;
        for (int k = 0; k <= 6; k++) {
          opt.idx[k] = dp[1][j].idx[k];
          opt.key[k] = dp[1][j].key[k];
        }
      }
    }
  }
  for (int i = 1; i < 7; i++) {
    // plr->keys[i] = opt.key[i] + avg;
    plr->keys[i] = static_cast<int>(opt.key[i]);
    plr->keys[i] += static_cast<float>(opt.key[i] - plr->keys[i]);
    plr->keys[i] += avg;

    plr->index[i - 1] = round(opt.idx[i]);
  }
}

/**
 * @brief predict the index of the next branch
 *
 * @param key
 * @return int index (from 0 to childNumber-1 )
 */
inline int PLRModel::Predict(double key) const {
  int s = 0;
  int e = 7;
  int mid;
  while (s < e) {
    mid = (s + e) >> 1;
    if (keys[mid] < key)
      s = mid + 1;
    else
      e = mid;
  }

  if (e == 0) {
    return 0;
  }

  int p;
  if (e == 7) {
    e = (flagNumber & 0x00FFFFFF) - 1;               // childNumber
    float a = (e - index[5]) / (keys[7] - keys[6]);  // a
    p = a * (key - keys[7]) + e;                     // b=e-a*keys[7], p=ax+b

    if (p > e) {
      return e;
    }

  } else if (e == 1) {
    float a = static_cast<float>(index[0]) / (keys[1] - keys[0]);
    p = a * (key - keys[0]);  // b = -a * keys[0], p=ax+b
  } else {
    float a = static_cast<float>(index[e - 1] - index[e - 2]) /
              (keys[e] - keys[e - 1]);
    p = a * (key - keys[e]) + index[e - 1];  // b=index[e-1]-a*keys[e]
  }
  if (p < 0) {
    p = 0;
  }
  return p;
}

#endif  // SRC_INCLUDE_NODES_INNERNODE_PLR_MODEL_H_
