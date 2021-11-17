/**
 * @file piecewiseLR.h
 * @author Jiaoyi
 * @brief piecewise linear regression model
 * @version 3.0
 * @date 2021-07-02
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef NODES_ROOTNODE_TRAINMODEL_PIECEWISELR_H_
#define NODES_ROOTNODE_TRAINMODEL_PIECEWISELR_H_

#include <float.h>
#include <math.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <utility>
#include <vector>

#include "../../../construct/structures.h"
#include "../../../params.h"
#include "../../innerNode/candidate_plr.h"

/**
 * @brief the piecewise linear regression model for root node
 *
 * @tparam DataVectorType the vector type of the dataset
 * @tparam KeyType the type of the key value
 */
template <typename DataVectorType, typename KeyType>
class PiecewiseLR {
 public:
  // *** Constructor

  /**
   * @brief Construct a new Piecewise LR object
   *
   * Input four data points into the point vector as the default value and set
   * the default value of the theta
   */
  PiecewiseLR() {
    for (int i = 0; i < SegmentNumber - 1; i++) {
      point.push_back({0, 0});
    }
    for (int i = 0; i < SegmentNumber; i++) {
      theta[i][0] = 0.0001;
      theta[i][1] = 0.666;
    }
    maxChildIdx = 2;
  }

 public:
  // *** Basic Public Functions of P. LR Model Objects

  /**
   * @brief use the given dataset to train the P. LR model
   *
   * @param[in] dataset the training dataset, each data point in it is composed
   * of the pair of the key value and the value.
   */
  void Train(const DataVectorType &dataset);

  /**
   * @brief output the unrounded index of the next node of the given key value
   *
   * @param[in] key the given key value
   * @return double: the unrounded index
   */
  inline double Predict(KeyType key) const;

 private:
  // *** Static Constant Options and Values of P. LR Model Objects

  /**
   * @brief The number of the segments.
   */
  static constexpr int SegmentNumber = 5;

 public:
  //*** Public Data Members of P. LR Model Objects

  /**
   * @brief The number of the child nodes
   */
  int maxChildIdx;

 private:
  //*** Private Data Members of P. LR Model Objects

  /**
   * @brief the breakpoints, each pair is: {the key value of this breakpoint,
   * the index of it}. Among them, the index is used as the index boundary of
   * each segment.
   */
  std::vector<std::pair<float, int>> point;

  /**
   * @brief The linear regression parameters of each segment. Each pair is: {the
   * slope, the intercept}.
   */
  float theta[SegmentNumber][2];
};

template <typename DataVectorType, typename KeyType>
void PiecewiseLR<DataVectorType, KeyType>::Train(
    const DataVectorType &dataset) {
  // construct the training dataset, x is the key value in the dataset, y is the
  // corresponding ratio of index in the maxChildIdx
  int size = dataset.size();
  if (size == 0) {
    return;
  }
  DataVectorType currdata = dataset;
  for (int i = 0; i < size; i++) {
    currdata[i].second = i * 1.0 / size * maxChildIdx;
  }

  // store the index and data points of candidate points into cand_index and
  // cand_point to speed up the dp algorithm
  int cand_size = std::min(size, 1000);
  DataVectorType cand_point(cand_size, {0, 0});
  std::vector<int> cand_index(cand_size, 0);
  CandidateCost<DataVectorType> cand_cost;
  float seg = size * 1.0 / cand_size;
  for (int i = 0; i < cand_size - 1; i++) {
    if (i * seg >= size) {
      for (; i < cand_size - 1; i++) {
        cand_index[i] = size - 1;
        cand_point[i] = currdata[size - 1];
      }
      break;
    }
    cand_index[i] = i * seg;
    cand_point[i] = currdata[i * seg];
  }
  cand_index[cand_size - 1] = size - 1;
  cand_point[cand_size - 1] = currdata[size - 1];
  cand_cost.StoreTheta(currdata, cand_index);

  SegmentPoint tmp;
  tmp.cost = -DBL_MAX;
  std::vector<SegmentPoint> tmpp(cand_size, tmp);
  std::vector<std::vector<SegmentPoint>> dp(2, tmpp);

  // initialize the dp[0]
  for (int j = 1; j < cand_size; j++) {
    dp[0][j].cost = cand_cost.Entropy(0, cand_index[j]);
    dp[0][j].key[0] = cand_point[0].first;
    dp[0][j].idx[0] = cand_index[0];
    dp[0][j].key[1] = cand_point[j].first;
    dp[0][j].idx[1] = cand_index[j];
    dp[0][j].key[SegmentNumber] = cand_point[cand_size - 1].first;
    dp[0][j].idx[SegmentNumber] = cand_index[cand_size - 1];
  }

  // use dp algorithm to calculate the optimal cost in each situation
  for (int i = 2; i < SegmentNumber; i++) {
    for (int j = i; j < cand_size - 1; j++) {
      SegmentPoint opt;
      opt.cost = -DBL_MAX;
      for (int k = i - 1; k < j; k++) {
        double res = -DBL_MAX;
        if (i < SegmentNumber - 1) {
          res = dp[0][k].cost + cand_cost.Entropy(cand_index[k], cand_index[j]);
        } else {
          res = dp[0][k].cost +
                cand_cost.Entropy(cand_index[k], cand_index[j]) +
                cand_cost.Entropy(cand_index[j], size - 1);
        }
        if (res > opt.cost) {
          opt.cost = res;
          for (int l = 0; l < i; l++) {
            opt.idx[l] = dp[0][k].idx[l];
            opt.key[l] = dp[0][k].key[l];
          }
          opt.key[i] = cand_point[j].first;
          opt.idx[i] = cand_index[j];
        }
      }
      dp[1][j].cost = opt.cost;
      for (int l = 0; l <= i; l++) {
        dp[1][j].idx[l] = opt.idx[l];
        dp[1][j].key[l] = opt.key[l];
      }
    }
    for (int m = 0; m < cand_size; m++) {
      dp[0][m].cost = dp[1][m].cost;
      for (int l = 0; l <= i + 1; l++) {
        dp[0][m].idx[l] = dp[1][m].idx[l];
        dp[0][m].key[l] = dp[1][m].key[l];
      }
    }
  }

  // find the optimal setting and store the parameters into point and theta
  SegmentPoint opt;
  opt.cost = -DBL_MAX;
  opt.idx[SegmentNumber] = cand_index[cand_size - 1];
  opt.key[SegmentNumber] = cand_point[cand_size - 1].first;
  for (int j = SegmentNumber; j < cand_size - 1; j++) {
    if (dp[1][j].cost > opt.cost) {
      opt.cost = dp[1][j].cost;
      for (int k = 0; k < SegmentNumber; k++) {
        opt.idx[k] = dp[1][j].idx[k];
        opt.key[k] = dp[1][j].key[k];
      }
    }
  }

  for (int i = 0; i < SegmentNumber - 1; i++) {
    point[i].first = opt.key[i + 1];
    point[i].second = currdata[opt.idx[i + 1]].second;
  }

  for (int i = 0; i < SegmentNumber; i++) {
    auto tmp_theta = cand_cost.theta.find({opt.idx[i], opt.idx[i + 1]});
    theta[i][0] = tmp_theta->second.first;
    theta[i][1] = tmp_theta->second.second;
  }
}

template <typename DataVectorType, typename KeyType>
inline double PiecewiseLR<DataVectorType, KeyType>::Predict(KeyType key) const {
  double p = 0;
  if (key <= point[0].first) {
    p = theta[0][0] * key + theta[0][1];
    if (p < 0)
      p = 0;
    else if (p > point[0].second)
      p = point[0].second;
  } else if (key <= point[SegmentNumber - 2].first) {
    for (int i = 1; i < SegmentNumber - 1; i++) {
      if (key <= point[i].first) {
        p = theta[i][0] * key + theta[i][1];
        if (p < point[i - 1].second + 1)
          p = point[i - 1].second + 1;
        else if (p > point[i].second)
          p = point[i].second;
        break;
      }
    }
  } else {
    p = theta[SegmentNumber - 1][0] * key + theta[SegmentNumber - 1][1];
    if (p < point[SegmentNumber - 2].second + 1)
      p = point[SegmentNumber - 2].second + 1;
    else if (p > maxChildIdx)
      p = maxChildIdx;
  }
  return p;
}

#endif  // NODES_ROOTNODE_TRAINMODEL_PIECEWISELR_H_
