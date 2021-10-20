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
  PiecewiseLR() {
    for (int i = 0; i < 4; i++) {
      point.push_back({-1, 0});
    }
    for (int i = 0; i < 5; i++) {
      theta[i][0] = 0.0001;
      theta[i][1] = 0.666;
    }
  }
  PiecewiseLR(const PiecewiseLR &currnode) {
    length = currnode.length;
    point = currnode.point;
    for (int i = 0; i < 5; i++) {
      theta[i][0] = currnode.theta[i][0];
      theta[i][1] = currnode.theta[i][1];
    }
  }
  /**
   * @brief train the P. LR model
   *
   * @param dataset the dataset
   */
  void Train(const DataVectorType &dataset) {
    // train the original dataset
    int size = dataset.size();
    int point_num = point.size() + 1;  // 5
    DataVectorType currdata = dataset;
    for (int i = 0; i < size; i++) {
      currdata[i].second = static_cast<double>(i) / size * length;
    }

    int cand_size = std::min(size, 1000);
    DataVectorType cand_point(cand_size, {0, 0});
    std::vector<int> cand_index(cand_size, 0);
    CandidateCost<DataVectorType, std::pair<KeyType, double>> cand_cost;
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

    for (int j = 1; j < cand_size; j++) {
      dp[0][j].cost = cand_cost.Entropy(0, cand_index[j]);
      dp[0][j].key[0] = cand_point[0].first;
      dp[0][j].idx[0] = cand_index[0];
      dp[0][j].key[1] = cand_point[j].first;
      dp[0][j].idx[1] = cand_index[j];
      dp[0][j].key[point_num] = cand_point[cand_size - 1].first;
      dp[0][j].idx[point_num] = cand_index[cand_size - 1];
    }

    for (int i = 2; i < point_num; i++) {
      for (int j = i + 1; j < cand_size - 1; j++) {
        SegmentPoint opt;
        opt.cost = -DBL_MAX;
        for (int k = i; k < j; k++) {
          double res = -DBL_MAX;
          if (i < point_num - 1) {
            res =
                dp[0][k].cost + cand_cost.Entropy(cand_index[k], cand_index[j]);
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

    SegmentPoint opt;
    opt.cost = -DBL_MAX;
    opt.idx[point_num] = cand_index[cand_size - 1];
    opt.key[point_num] = cand_point[cand_size - 1].first;
    for (int j = point_num; j < cand_size - 1; j++) {
      if (dp[1][j].cost > opt.cost) {
        opt.cost = dp[1][j].cost;
        for (int k = 0; k < point_num; k++) {
          opt.idx[k] = dp[1][j].idx[k];
          opt.key[k] = dp[1][j].key[k];
        }
      }
    }

    for (int i = 0; i < point_num - 1; i++) {
      point[i].first = opt.key[i + 1];
      point[i].second = currdata[opt.idx[i + 1]].second;
    }

    for (int i = 0; i < point_num; i++) {
      auto tmp_theta = cand_cost.theta.find({opt.idx[i], opt.idx[i + 1]});
      theta[i][0] = tmp_theta->second.first;
      theta[i][1] = tmp_theta->second.second;
    }
  }

  /**
   * @brief predict the next node of the given key value
   *
   * @param key the given key value
   * @return int: the index of the next node
   */
  int Predict(double key) const {
    int p = PredictIdx(key);
    return p;
  }

  /**
   * @brief output the unrounded index
   *
   * @param key the given key value
   * @return double: the index
   */
  inline double PredictIdx(double key) const {
    double p = 0;
    if (key <= point[0].first) {
      p = theta[0][0] * key + theta[0][1];
      if (p < 0)
        p = 0;
      else if (p > point[0].second)
        p = point[0].second;
    } else if (key <= point[1].first) {
      p = theta[1][0] * key + theta[1][1];
      if (p < point[0].second + 1)
        p = point[0].second + 1;
      else if (p > point[1].second)
        p = point[1].second;
    } else if (key <= point[2].first) {
      p = theta[2][0] * key + theta[2][1];
      if (p < point[1].second + 1)
        p = point[1].second + 1;
      else if (p > point[2].second)
        p = point[2].second;
    } else if (key <= point[3].first) {
      p = theta[3][0] * key + theta[3][1];
      if (p < point[2].second + 1)
        p = point[2].second + 1;
      else if (p > point[3].second)
        p = point[3].second;
    } else {
      p = theta[4][0] * key + theta[4][1];
      if (p < point[3].second + 1)
        p = point[3].second + 1;
      else if (p > length)
        p = length;
    }
    return p;
  }

  int length;  ///< the number of child nodes

 private:
  std::vector<std::pair<float, int>> point;  ///< <key, boundary>
  float theta[5][2];                         ///< {theta1, theta2}
};
#endif  // NODES_ROOTNODE_TRAINMODEL_PIECEWISELR_H_
