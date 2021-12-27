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
 */
class PrefetchPLR {
 public:
  /**
   * @brief Construct a new Prefetch PLR object
   *
   * Input nine data points into the point vector as the default value and set
   * the default value of the theta
   */
  PrefetchPLR() {
    blockNumber = 1;
    for (int i = 0; i < SegmentNumber - 1; i++) {
      point.push_back({-1, 0});
    }
    for (int i = 0; i < SegmentNumber; i++) {
      theta[i][0] = 1;
      theta[i][1] = 1;
    }
  }

 public:
  // *** Basic Public Functions of Prefetch Prediction Model Objects

  /**
   * @brief Set the Block Number object
   *
   * @param[in] cnt the new number of data blocks
   */
  void SetBlockNumber(int cnt) { blockNumber = cnt; }

  /**
   * @brief train this prefetch prediction model
   *
   * @param[in] dataset the dataset used to train this model, each pair is: {the
   * unrounded leaf node index of each key value, the index of the data block}
   * @param[in] LeafCost the cost of each leaf node, each member is: {the
   * rounded leaf node index, the costs of this node in different number of data
   * blocks}
   * @return int: the new blockNumber
   */
  int PrefetchTrain(const std::vector<std::pair<double, int>> &dataset,
                    const std::map<int, std::vector<double>> &LeafCost);

  /**
   * @brief predict the index of the data block according to the given unrounded
   * leaf node index
   *
   * @param[in] unroundedLeafIdx the given unrounded leaf node index
   * @return int: the index of the data block
   */
  int PrefetchPredict(double unroundedLeafIdx) const;

  /**
   * @brief get the slope of the corresponding segment, which is also the number
   * of data blocks of a leaf node
   *
   * @param[in] unroundedLeafIdx the given unrounded leaf node index
   * @return int: the slope, which is also the number of the allocated data
   * blocks
   */
  int PrefetchNum(double unroundedLeafIdx) const;

 private:
  // *** Private Functions of Prefetch Prediction Model Objects

  /**
   * @brief Get the optimal cost of this segment in different numbers of
   * allocated data blocks, whose range is [leftCandidateID, rightCandidateID)
   *
   * @param[in] CostCandidate the vector used to store costs of the candidate
   * points, each is the vector of cost in different numbers of allocated data
   * blocks
   * @param[in] leftCandidateID the leaf index in CostCandidate
   * @param[in] rightCandidateID the right index in CostCandidate
   * @param[in] maxBlockNum the maximum number of data blocks
   * @return std::pair<int, double>: {optimal block number, optimal cost}
   */
  inline std::pair<int, double> GetCost(
      const std::vector<std::vector<double>> &CostCandidate,
      int leftCandidateID, int rightCandidateID, int maxBlockNum);

 private:
  // *** Static Constant Options and Values of Prefetch Prediction Model Objects

  /**
   * @brief The number of the segments.
   */
  static constexpr int SegmentNumber = 10;

 private:
  //*** Private Data Members of Prefetch Prediction Model Model Objects

  /**
   * @brief The number of the data blocks
   */
  int blockNumber;

  /**
   * @brief the breakpoints, each pair is: {the rounded index of this leaf node,
   * the index of the data block}. Among them, y is used as the index
   * boundary of each segment.
   */
  std::vector<std::pair<int, int>> point;

  /**
   * @brief The linear regression parameters of each segment. Each pair is: {the
   * slope, the intercept}.
   *
   * Here we force the slope and intercept of each segment to be integers, so
   * that within each segment, each leaf node is mapped to the same number of
   * data blocks. Thus, the slope is the allocated number of data blocks for the
   * leaf node in this segment.
   */
  int theta[SegmentNumber][2];
};

inline int PrefetchPLR::PrefetchTrain(
    const std::vector<std::pair<double, int>> &dataset,
    const std::map<int, std::vector<double>> &LeafCost) {
  int size = dataset.size();

  int candidate_size = std::min(50, size);
  int seg_len = size / candidate_size;

  // initialize the candidates
  std::vector<std::pair<double, int>> candidates;
  for (int i = 0; i < size; i += seg_len) {
    candidates.push_back(dataset[i]);
    if (static_cast<int>(candidates.size()) == candidate_size) {
      break;
    }
  }
  candidates.push_back(dataset[size - 1]);
  candidate_size = candidates.size();

  // initialize the cost of each candidate point
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

  // initialize the dp[0]
  std::vector<std::vector<SegmentPoint<double>>> dp(
      SegmentNumber, std::vector<SegmentPoint<double>>(candidate_size));
  for (int i = 0; i < candidate_size; i++) {
    auto res = GetCost(CostCandidate, 0, i, maxBlockNum);
    dp[1][i].cost = res.second;
    dp[1][i].blockNum[1] = res.first;
    dp[1][i].key[0] = dataset[0].first;
    dp[1][i].idx[0] = 0;
    dp[1][i].key[1] = candidates[i].first;
    dp[1][i].idx[1] = i;
  }

  // use dp algorithm to calculate the optimal cost in each situation
  for (int i = 2; i < SegmentNumber - 1; i++) {
    for (int j = i; j < candidate_size; j++) {
      SegmentPoint<double> opt;
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

  for (int j = SegmentNumber - 1; j < candidate_size; j++) {
    SegmentPoint<double> opt;
    opt.cost = DBL_MAX;
    for (int k = SegmentNumber - 2; k < j; k++) {
      double tmp_cost = dp[SegmentNumber - 2][k].cost;

      auto res = GetCost(CostCandidate, k, j, maxBlockNum);
      tmp_cost += res.second;
      int tmp_block0 = res.first;
      res = GetCost(CostCandidate, j, candidate_size - 1, maxBlockNum);
      tmp_cost += res.second;
      int tmp_block1 = res.first;
      if (tmp_cost < opt.cost) {
        opt.cost = tmp_cost;
        for (int l = 0; l < SegmentNumber - 1; l++) {
          opt.idx[l] = dp[SegmentNumber - 2][k].idx[l];
          opt.key[l] = dp[SegmentNumber - 2][k].key[l];
          opt.blockNum[l] = dp[SegmentNumber - 2][k].blockNum[l];
        }
        opt.key[SegmentNumber - 1] = candidates[j].first;
        opt.idx[SegmentNumber - 1] = j;
        opt.idx[SegmentNumber] = candidate_size - 1;
        opt.blockNum[SegmentNumber - 1] = tmp_block0;
        opt.blockNum[SegmentNumber] = tmp_block1;
      }
    }
    dp[SegmentNumber - 1][j].cost = opt.cost;
    for (int l = 0; l <= SegmentNumber; l++) {
      dp[SegmentNumber - 1][j].idx[l] = opt.idx[l];
      dp[SegmentNumber - 1][j].key[l] = opt.key[l];
      dp[SegmentNumber - 1][j].blockNum[l] = opt.blockNum[l];
    }
  }

  // find the optimal setting and store the parameters
  SegmentPoint<double> opt;
  opt.cost = DBL_MAX;
  for (int i = SegmentNumber - 1; i < candidate_size; i++) {
    if (dp[SegmentNumber - 1][i].cost < opt.cost) {
      opt.cost = dp[SegmentNumber - 1][i].cost;
      for (int l = 0; l <= SegmentNumber; l++) {
        opt.idx[l] = dp[SegmentNumber - 1][i].idx[l];
        opt.key[l] = dp[SegmentNumber - 1][i].key[l];
        opt.blockNum[l] = dp[SegmentNumber - 1][i].blockNum[l];
      }
    }
  }

  for (int i = 0; i < SegmentNumber - 1; i++) {
    point[i].first = opt.key[i + 1];
    point[i].second = candidates[opt.idx[i + 1]].second;
  }

  theta[0][0] = opt.blockNum[1];
  theta[0][1] =
      -theta[0][0] * static_cast<double>(candidates[opt.idx[0]].first);
  point[0].second =
      theta[0][0] * static_cast<double>(point[0].first) + theta[0][1];
  for (int i = 1; i < SegmentNumber; i++) {
    theta[i][0] = opt.blockNum[i + 1];
    theta[i][1] = static_cast<double>(point[i - 1].second) -
                  theta[i][0] * static_cast<double>(point[i - 1].first);
    if (i < SegmentNumber - 1) {
      point[i].second =
          theta[i][0] * static_cast<double>(point[i].first) + theta[i][1];
    }
  }
  theta[SegmentNumber - 1][0] = opt.blockNum[SegmentNumber];
  theta[SegmentNumber - 1][1] =
      point[SegmentNumber - 2].second -
      theta[SegmentNumber - 1][0] *
          static_cast<double>(point[SegmentNumber - 2].first);
  blockNumber = theta[SegmentNumber - 1][0] *
                    static_cast<double>(dataset[size - 1].first) +
                theta[SegmentNumber - 1][1] + 1;

  return blockNumber;
}

inline int PrefetchPLR::PrefetchPredict(double unroundedLeafIdx) const {
  int s = 0;
  int e = SegmentNumber - 1;
  int mid;
  // first perform a binary search among the keys
  while (s < e) {
    mid = (s + e) >> 1;
    if (point[mid].first <= unroundedLeafIdx)
      s = mid + 1;
    else
      e = mid;
  }

  int p = 0;
  if (s == 0) {
    p = theta[0][0] * unroundedLeafIdx + theta[0][1];
    if (p < 0)
      p = 0;
    else if (p > point[0].second - 1)
      p = point[0].second - 1;
  } else if (s < SegmentNumber - 1) {
    p = theta[s][0] * unroundedLeafIdx + theta[s][1];
    if (p < point[s - 1].second)
      p = point[s - 1].second;
    else if (p > point[s].second - 1)
      p = point[s].second - 1;
  } else {
    p = theta[SegmentNumber - 1][0] * unroundedLeafIdx +
        theta[SegmentNumber - 1][1];
    if (p < point[SegmentNumber - 2].second)
      p = (point[SegmentNumber - 2].second);
    else if (p > blockNumber - 1)
      p = blockNumber - 1;
  }
  return p;
}

inline int PrefetchPLR::PrefetchNum(double unroundedLeafIdx) const {
  int p = 0;
  if (unroundedLeafIdx < point[0].first) {
    p = theta[0][0];

  } else if (unroundedLeafIdx < point[SegmentNumber - 2].first) {
    for (int j = 1; j < SegmentNumber - 1; j++) {
      if (unroundedLeafIdx < point[j].first) {
        p = theta[j][0];
        break;
      }
    }
  } else {
    p = theta[SegmentNumber - 1][0];
  }
  return p;
}

inline std::pair<int, double> PrefetchPLR::GetCost(
    const std::vector<std::vector<double>> &CostCandidate, int leftCandidateID,
    int rightCandidateID, int maxBlockNum) {
  double opt_cost = DBL_MAX;
  int opt_block = 1;
  for (int i = 0; i < maxBlockNum; i++) {
    double cost =
        CostCandidate[rightCandidateID][i] - CostCandidate[leftCandidateID][i];
    if (cost < opt_cost) {
      opt_cost = cost;
      opt_block = i + 1;
    }
  }
  return {opt_block, opt_cost};
}

#endif  // NODES_ROOTNODE_TRAINMODEL_PREFETCH_PLR_H_
