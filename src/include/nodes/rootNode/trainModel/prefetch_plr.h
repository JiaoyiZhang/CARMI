/**
 * @file prefetch_plr.h
 * @author Jiaoyi
 * @brief the prefetching model
 * @version 3.0
 * @date 2021-07-02
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef NODES_ROOTNODE_TRAINMODEL_PREFETCH_PLR_H_
#define NODES_ROOTNODE_TRAINMODEL_PREFETCH_PLR_H_

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
 * @brief the prefetch prediction model
 *
 * @tparam DataVectorType the vector type of the dataset
 * @tparam KeyType the type of the key value
 */
template <typename DataVectorType, typename keyType>
class PrefetchPLR {
 public:
  PrefetchPLR() {
    blockNumber = 1;
    for (int i = 0; i < 9; i++) {
      point.push_back({-1, 0});
    }
    for (int i = 0; i < 10; i++) {
      theta[i][0] = 0.0001;
      theta[i][1] = 0.666;
    }
  }
  PrefetchPLR(const PrefetchPLR &currnode) {
    blockNumber = 1;
    point = currnode.point;
    for (int i = 0; i < 10; i++) {
      theta[i][0] = currnode.theta[i][0];
      theta[i][1] = currnode.theta[i][1];
    }
  }
  /**
   * @brief Set the Block Number object
   *
   * @param cnt the new number of data blocks
   */
  void SetBlockNumber(int cnt) { blockNumber = cnt; }

  /**
   * @brief train this model
   *
   * @param dataset the dataset
   * @param LeafCost the cost of each leaf node
   * @param checkpoint
   * @return int: the new blockNumber
   */
  int PrefetchTrain(const DataVectorType &dataset,
                    const std::map<int, std::vector<double>> &LeafCost,
                    int checkpoint) {
    int size = dataset.size();
    int point_num = point.size() + 1;

    int candidate_size = std::min(50, size);
    int seg_len = size / candidate_size;

    DataVectorType candidates;
    std::map<double, int> idxInData;  // leaf id, index in dataset
    if (checkpoint != -1) {
      candidates.push_back(dataset[checkpoint]);
      idxInData.insert({dataset[checkpoint].first, checkpoint});
    }
    for (int i = 0; i < size; i += seg_len) {
      candidates.push_back(dataset[i]);
      idxInData.insert({dataset[i].first, i});
      if (candidates.size() == candidate_size) {
        break;
      }
    }
    candidates.push_back(dataset[size - 1]);
    idxInData.insert({dataset[size - 1].first, size - 1});
    candidate_size = candidates.size();
    std::sort(candidates.begin(), candidates.end());

    std::vector<std::vector<double>> CostCandidate(candidate_size);
    int maxBlockNum = LeafCost.begin()->second.size();
    CostCandidate[0] = std::vector<double>(maxBlockNum, 0.0);
    for (int i = 1; i < candidate_size; i++) {
      std::vector<double> cost = CostCandidate[i - 1];
      for (int j = 0; j < maxBlockNum; j++) {
        for (int k = candidates[i - 1].first; k < candidates[i].first; k++) {
          auto tmp_it = LeafCost.find(k);
          if (tmp_it != LeafCost.end()) {
            auto tmp_leaf_cost = tmp_it->second;
            cost[j] += tmp_leaf_cost[j];
          }
        }
      }
      CostCandidate[i] = cost;
    }

    std::vector<SegmentPoint> tmp(candidate_size);
    std::vector<std::vector<SegmentPoint>> dp(point_num, tmp);
    for (int i = 0; i < candidate_size; i++) {
      auto res = GetCost(CostCandidate, 0, i, maxBlockNum);
      dp[1][i].cost = res.second;
      dp[1][i].blockNum[1] = res.first;
      dp[1][i].key[0] = dataset[0].first;
      dp[1][i].idx[0] = 0;
      dp[1][i].key[1] = candidates[i].first;
      dp[1][i].idx[1] = i;
    }

    for (int i = 2; i < point_num - 1; i++) {
      for (int j = i; j < candidate_size; j++) {
        SegmentPoint opt;
        opt.cost = DBL_MAX;
        for (int k = i - 1; k < j; k++) {
          double tmp_cost = dp[i - 1][k].cost;
          auto res = GetCost(CostCandidate, k, j, maxBlockNum);
          tmp_cost += res.second;
          int tmp_block = res.first;
          if (tmp_cost < opt.cost) {
            opt.cost = tmp_cost;
            for (int l = 0; l < i; l++) {
              opt.idx[l] = dp[i - 1][k].idx[l];
              opt.key[l] = dp[i - 1][k].key[l];
              opt.blockNum[l] = dp[i - 1][k].blockNum[l];
            }
            opt.key[i] = candidates[j].first;
            opt.idx[i] = j;
            opt.blockNum[i] = tmp_block;
          }
        }
        dp[i][j].cost = opt.cost;
        for (int l = 0; l <= i; l++) {
          dp[i][j].idx[l] = opt.idx[l];
          dp[i][j].key[l] = opt.key[l];
          dp[i][j].blockNum[l] = opt.blockNum[l];
        }
      }
    }

    for (int j = point_num - 1; j < candidate_size; j++) {
      SegmentPoint opt;
      opt.cost = DBL_MAX;
      for (int k = point_num - 2; k < j; k++) {
        double tmp_cost = dp[point_num - 2][k].cost;

        auto res = GetCost(CostCandidate, k, j, maxBlockNum);
        tmp_cost += res.second;
        int tmp_block0 = res.first;
        res = GetCost(CostCandidate, j, candidate_size - 1, maxBlockNum);
        tmp_cost += res.second;
        int tmp_block1 = res.first;
        if (tmp_cost < opt.cost) {
          opt.cost = tmp_cost;
          for (int l = 0; l < point_num - 1; l++) {
            opt.idx[l] = dp[point_num - 2][k].idx[l];
            opt.key[l] = dp[point_num - 2][k].key[l];
            opt.blockNum[l] = dp[point_num - 2][k].blockNum[l];
          }
          opt.key[point_num - 1] = candidates[j].first;
          opt.idx[point_num - 1] = j;
          opt.idx[point_num] = candidate_size - 1;
          opt.blockNum[point_num - 1] = tmp_block0;
          opt.blockNum[point_num] = tmp_block1;
        }
      }
      dp[point_num - 1][j].cost = opt.cost;
      for (int l = 0; l <= point_num; l++) {
        dp[point_num - 1][j].idx[l] = opt.idx[l];
        dp[point_num - 1][j].key[l] = opt.key[l];
        dp[point_num - 1][j].blockNum[l] = opt.blockNum[l];
      }
    }

    SegmentPoint opt;
    opt.cost = DBL_MAX;
    for (int i = point_num - 1; i < candidate_size; i++) {
      if (dp[point_num - 1][i].cost < opt.cost) {
        opt.cost = dp[point_num - 1][i].cost;
        for (int l = 0; l <= point_num; l++) {
          opt.idx[l] = dp[point_num - 1][i].idx[l];
          opt.key[l] = dp[point_num - 1][i].key[l];
          opt.blockNum[l] = dp[point_num - 1][i].blockNum[l];
        }
      }
    }

    for (int i = 0; i < point_num - 1; i++) {
      point[i].first = opt.key[i + 1];
      point[i].second = candidates[opt.idx[i + 1]].second;
    }

    theta[0][0] = opt.blockNum[1];
    theta[0][1] = -theta[0][0] * candidates[opt.idx[0]].first;
    point[0].second = theta[0][0] * point[0].first + theta[0][1];
    for (int i = 1; i < point_num; i++) {
      theta[i][0] = opt.blockNum[i + 1];
      theta[i][1] = point[i - 1].second - theta[i][0] * point[i - 1].first;
      if (i < point_num - 1) {
        point[i].second = theta[i][0] * point[i].first + theta[i][1];
      }
    }
    theta[point_num - 1][0] = opt.blockNum[point_num];
    theta[point_num - 1][1] =
        point[point_num - 2].second -
        theta[point_num - 1][0] * point[point_num - 2].first;
    blockNumber = theta[point_num - 1][0] * dataset[size - 1].first +
                  theta[point_num - 1][1] + 1;

    return blockNumber;
  }
  /**
   * @brief Get the optimal cost of this segment
   *
   * @param CostCandidate the vector used to store cost
   * @param leftCandidateID the leaf index
   * @param rightCandidateID the right index
   * @param maxBlockNum the maximum number of data blocks
   * @return std::pair<int, double>: {opt_block, opt_cost}
   */
  inline std::pair<int, double> GetCost(
      const std::vector<std::vector<double>> &CostCandidate,
      int leftCandidateID, int rightCandidateID, int maxBlockNum) {
    double opt_cost = DBL_MAX;
    int opt_block = 1;
    for (int i = 0; i < maxBlockNum; i++) {
      double cost = CostCandidate[rightCandidateID][i] -
                    CostCandidate[leftCandidateID][i];
      if (cost < opt_cost) {
        opt_cost = cost;
        opt_block = i + 1;
      }
    }
    return {opt_block, opt_cost};
  }
  /**
   * @brief predict the index of the data block
   * where the key value is stored
   *
   * @param key the given key value
   * @return int: the index of the data block
   */
  int PrefetchPredict(double key) const {
    int p = 0;
    if (key < point[0].first) {
      p = theta[0][0] * key + theta[0][1];
      if (p < 0)
        p = 0;
      else if (p > point[0].second - 1)
        p = point[0].second - 1;

    } else if (key < point[8].first) {
      for (int j = 1; j <= 8; j++) {
        if (key < point[j].first) {
          p = theta[j][0] * key + theta[j][1];
          if (p < point[j - 1].second)
            p = point[j - 1].second;
          else if (p > point[j].second - 1)
            p = point[j].second - 1;
          return p;
        }
      }

    } else {
      p = theta[9][0] * key + theta[9][1];
      if (p < point[8].second)
        p = (point[8].second);
      else if (p > blockNumber - 1)
        p = blockNumber - 1;
    }
    return p;
  }

  /**
   * @brief get the slope of the corresponding segment
   * which is also the number of data blocks of a
   * leaf node
   *
   * @param key the given key value
   * @return int: the slope
   */
  int PrefetchNum(double key) const {
    int p = 0;
    if (key < point[0].first) {
      p = theta[0][0];

    } else if (key < point[8].first) {
      for (int j = 1; j <= 8; j++) {
        if (key < point[j].first) {
          p = theta[j][0];
          return p;
        }
      }
    } else {
      p = theta[9][0];
    }
    return p;
  }

 private:
  int blockNumber;                         ///< the number of all data blocks
  std::vector<std::pair<int, int>> point;  ///< <key, boundary>
  int theta[10][2];                        ///< {slope, intercept}
};
#endif  // NODES_ROOTNODE_TRAINMODEL_PREFETCH_PLR_H_
