/**
 * @file candidate_plr.h
 * @author Jiaoyi
 * @brief class for piecewise linear regression model
 * @version 3.0
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <math.h>

#include <map>
#include <utility>
#include <vector>

#include "../../construct/structures.h"
#include "../../params.h"

#ifndef NODES_INNERNODE_CANDIDATE_PLR_H_
#define NODES_INNERNODE_CANDIDATE_PLR_H_

/**
 * @brief designed for P. LR model
 *
 */
struct SegmentPoint {
  /**
   * @brief the current cost
   *
   */
  double cost;

  /**
   * @brief the key values
   *
   */
  double key[12] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

  /**
   * @brief the corresponding indexes
   *
   */
  int idx[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  /**
   * @brief the number of blocks
   *
   */
  int blockNum[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
};

/**
 * @brief designed for P. LR model
 *
 * @tparam DataVectorType the vector type of the dataset
 * @tparam DataType the type of a data point
 */
template <typename DataVectorType, typename DataType>
class CandidateCost {
 public:
  CandidateCost() {}
  /**
   * @brief store params for each segment
   *
   * @param dataset the dataset
   * @param index the indexes of candidates
   */
  void StoreTheta(const DataVectorType &dataset,
                  const std::vector<int> &index) {
    StoreValue(dataset, index);
    for (int i = 0; i < index.size() - 1; i++) {
      for (int j = i + 1; j < index.size(); j++) {
        int l = index[i];
        int r = index[j];
        auto tmp_theta = TrainLR(i, j, r - l);
        theta.insert({{l, r}, tmp_theta});
      }
    }
  }

  /**
   * @brief calculate the entropy of each segment
   *
   * @param leftIdx the left index of the sub-dataset
   * @param rightIdx the right-index of the sub-dataset
   * @return double: entropy
   */
  double Entropy(int leftIdx, int rightIdx) {
    auto tmp_theta = theta.find({leftIdx, rightIdx});
    double a = tmp_theta->second.first;
    double entropy = log2(a) * (rightIdx - leftIdx);
    return entropy;
  }

  /**
   * @brief train the params of the segment
   *
   * @param left the left index of the sub-dataset
   * @param right the right index of the sub-dataset
   * @param size the size of the sub-dataset
   * @return std::pair<double, double>: the slope and intercept
   */
  std::pair<double, double> TrainLR(int left, int right, int size) {
    double theta1 = 0.0001, theta2 = 0.666;
    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    t1 = xx[right] - xx[left];
    t2 = x[right] - x[left];
    t3 = px[right] - px[left];
    t4 = p[right] - p[left];

    theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);

    return {theta1, theta2};
  }

  /**
   * @brief store the value of each segment
   *
   * @param dataset the original dataset
   * @param index the corresponding indexes in the dataset
   */
  void StoreValue(const DataVectorType &dataset,
                  const std::vector<int> &index) {
    xx = std::vector<double>(index.size(), 0);
    x = std::vector<double>(index.size(), 0);
    px = std::vector<double>(index.size(), 0);
    p = std::vector<double>(index.size(), 0);
    xx[0] = 0.0;
    x[0] = 0.0;
    px[0] = 0.0;
    p[0] = 0.0;
    for (int i = 1; i < static_cast<int>(index.size()); i++) {
      for (int j = index[i - 1]; j < index[i]; j++) {
        xx[i] += dataset[j].first * dataset[j].first;
        x[i] += dataset[j].first;
        px[i] += dataset[j].first * dataset[j].second;
        p[i] += dataset[j].second;
      }
      xx[i] += xx[i - 1];
      x[i] += x[i - 1];
      px[i] += px[i - 1];
      p[i] += p[i - 1];
    }
  }

 public:
  /**
   * @brief params for the corresponding segment
   *
   */
  std::map<std::pair<int, int>, std::pair<double, double>> theta;

  /**
   * @brief  the value of key*key
   *
   */
  std::vector<double> xx;

  /**
   * @brief the value of key
   *
   */
  std::vector<double> x;

  /**
   * @brief the value of y*key
   *
   */
  std::vector<double> px;

  /**
   * @brief the value of y
   *
   */
  std::vector<double> p;
};

#endif  // NODES_INNERNODE_CANDIDATE_PLR_H_
