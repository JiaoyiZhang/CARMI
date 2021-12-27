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
    theta = new float *[SegmentNumber];
    for (int i = 0; i < SegmentNumber; i++) {
      theta[i] = new float[2];
      theta[i][0] = 0.0001;
      theta[i][1] = 0.666;
    }
    minValue = 0;
    maxChildIdx = 2;
  }

#ifdef DEBUG
  int GetSegNum() { return SegmentNumber; }

  std::vector<std::pair<float, int>> GetPoint() { return point; }
#endif  // DEBUG

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

 public:
  //*** Public Data Members of P. LR Model Objects

  /**
   * @brief The number of the child nodes
   */
  int maxChildIdx;

 private:
  //*** Private Data Members of P. LR Model Objects

  /**
   * @brief The number of the segments.
   */
  int SegmentNumber = 5;

  /**
   * @brief the breakpoints, each pair is: {the key value of this breakpoint,
   * the index of it}. Among them, the index is used as the index boundary of
   * each segment.
   */
  std::vector<std::pair<KeyType, int>> point;

  /**
   * @brief The minimum value.
   */
  KeyType minValue;

  /**
   * @brief The linear regression parameters of each segment. Each pair is: {the
   * slope, the intercept}.
   */
  float **theta;
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
  typedef std::vector<std::pair<KeyType, double>> TrainType;
  TrainType currdata(dataset.size());
  minValue = dataset[0].first;
  for (int i = 0; i < size; i++) {
    currdata[i].first = dataset[i].first - minValue;
    currdata[i].second = static_cast<double>(i) / size * maxChildIdx;
  }

  // store the index and data points of candidate points into cand_index and
  // cand_point to speed up the dp algorithm
  int cand_size = std::min(size, 1000);
  TrainType cand_point(cand_size, {0, 0});
  std::vector<int> cand_index(cand_size, 0);
  CandidateCost<TrainType> cand_cost;
  float seg = static_cast<double>(size) / cand_size;
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

  float plr_cost =
      cand_cost.Entropy(0, size - 1, 0, cand_point[cand_size - 1].second);
  point[0].first = static_cast<KeyType>(DBL_MAX);
  point[0].second = maxChildIdx;
  auto tmp_theta = cand_cost.theta.find({0, size - 1});
  theta[0][0] = tmp_theta->second.first;
  theta[0][1] = tmp_theta->second.second;
  int opt_seg = 1;

  for (int seg = 2; seg <= SegmentNumber; seg++) {
    std::vector<std::vector<SegmentPoint<KeyType>>> dp(
        2,
        std::vector<SegmentPoint<KeyType>>(cand_size, SegmentPoint<KeyType>()));

    // initialize the dp[0]
    for (int j = 1; j < cand_size; j++) {
      dp[0][j].cost =
          cand_cost.Entropy(0, cand_index[j], 0, cand_point[j].second);
      dp[0][j].key[0] = cand_point[0].first;
      dp[0][j].idx[0] = cand_index[0];
      dp[0][j].key[1] = cand_point[j].first;
      dp[0][j].idx[1] = cand_index[j];
      dp[0][j].key[seg] = cand_point[cand_size - 1].first;
      dp[0][j].idx[seg] = cand_index[cand_size - 1];
    }

    // use dp algorithm to calculate the optimal cost in each situation
    for (int i = 2; i < seg; i++) {
      for (int j = i; j < cand_size - 1; j++) {
        SegmentPoint<KeyType> opt;
        for (int k = i - 1; k < j; k++) {
          float res = -DBL_MAX;
          if (i < seg - 1) {
            res = dp[0][k].cost +
                  cand_cost.Entropy(cand_index[k], cand_index[j],
                                    cand_point[k].second, cand_point[j].second);
          } else {
            res =
                dp[0][k].cost +
                cand_cost.Entropy(cand_index[k], cand_index[j],
                                  cand_point[k].second, cand_point[j].second) +
                cand_cost.Entropy(cand_index[j], size - 1, cand_point[j].second,
                                  cand_point[cand_size - 1].second);
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
    SegmentPoint<KeyType> opt;
    opt.idx[seg] = cand_index[cand_size - 1];
    opt.key[seg] = cand_point[cand_size - 1].first;
    for (int j = seg; j < cand_size - 1; j++) {
      if (dp[1][j].cost > opt.cost) {
        opt.cost = dp[1][j].cost;
        for (int k = 0; k < seg; k++) {
          opt.idx[k] = dp[1][j].idx[k];
          opt.key[k] = dp[1][j].key[k];
        }
      }
    }
    if (opt.cost > plr_cost) {
      plr_cost = opt.cost;
      opt_seg = seg;
      for (int i = 0; i < seg - 1; i++) {
        point[i].first = opt.key[i + 1];
        point[i].second = currdata[opt.idx[i + 1]].second;
      }

      for (int i = 0; i < seg; i++) {
        auto tmp_theta = cand_cost.theta.find({opt.idx[i], opt.idx[i + 1]});
        theta[i][0] = tmp_theta->second.first;
        theta[i][1] = tmp_theta->second.second;
      }
    }
  }
  SegmentNumber = opt_seg;
  if (SegmentNumber < 2) {
    point[0] = {static_cast<KeyType>(DBL_MAX), maxChildIdx};
  }
}

template <typename DataVectorType, typename KeyType>
inline double PiecewiseLR<DataVectorType, KeyType>::Predict(KeyType key) const {
  KeyType tmpKey = key - minValue;
  double p = 0;
  if (tmpKey <= point[0].first) {
    p = theta[0][0] * static_cast<double>(tmpKey) + theta[0][1];
    if (p < 0)
      p = 0;
    else if (p > point[0].second)
      p = point[0].second;
  } else if (tmpKey <= point[SegmentNumber - 2].first) {
    for (int i = 1; i < SegmentNumber - 1; i++) {
      if (tmpKey <= point[i].first) {
        p = theta[i][0] * static_cast<double>(tmpKey) + theta[i][1];
        if (p < point[i - 1].second + 1)
          p = point[i - 1].second + 1;
        else if (p > point[i].second)
          p = point[i].second;
        break;
      }
    }
  } else {
    p = theta[SegmentNumber - 1][0] * static_cast<double>(tmpKey) +
        theta[SegmentNumber - 1][1];
    if (p < point[SegmentNumber - 2].second + 1)
      p = point[SegmentNumber - 2].second + 1;
    else if (p > maxChildIdx)
      p = maxChildIdx;
  }
  return p;
}

#endif  // NODES_ROOTNODE_TRAINMODEL_PIECEWISELR_H_
