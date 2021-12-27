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
 * @brief Designed for the piecewise linear regression model. This structure
 * records all the contents that need to be stored in the training process of
 * the piecewise linear function, which is the item in the dp table.
 */
template <typename KeyType>
struct SegmentPoint {
  /**
   * @brief the current cost
   */
  float cost = -DBL_MAX;

  /**
   * @brief the key values
   */
  KeyType key[12] = {KeyType(), KeyType(), KeyType(), KeyType(),
                     KeyType(), KeyType(), KeyType(), KeyType(),
                     KeyType(), KeyType(), KeyType(), KeyType()};

  /**
   * @brief the corresponding indexes
   */
  int idx[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  /**
   * @brief the number of blocks for the dp table in the prefetch prediction
   * model
   */
  int blockNum[12] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
};

/**
 * @brief designed for piecewise linear regression model.
 *
 * This class stores the cost between each candidate point, the parameters of
 * the line segment between two points, entropy, and so on in the process of
 * dynamic programming algorithm in P. LR model, which is used to assist and
 * accelerate the DP algorithm.
 *
 * @tparam DataVectorType the vector type of the dataset, each element is a
 * pair: {key, value}
 */
template <typename DataVectorType>
class CandidateCost {
 public:
  /**
   * @brief Construct a new empty Candidate Cost object
   */
  CandidateCost() {}

  /**
   * @brief store the slope and intercept of each segment
   *
   * @param[in] dataset the given dataset, each element is: {key value, y}
   * @param[in] index the indexes of candidates
   */
  void StoreTheta(const DataVectorType &dataset,
                  const std::vector<int> &index) {
    // store the value of each segment for least squares, used to speed up the
    // training process of the linear regression
    std::vector<long double> xx(index.size(), 0);
    std::vector<long double> x(index.size(), 0);
    std::vector<long double> px(index.size(), 0);
    std::vector<long double> p(index.size(), 0);
    xx[0] = 0.0;
    x[0] = 0.0;
    px[0] = 0.0;
    p[0] = 0.0;
    for (int i = 1; i < static_cast<int>(index.size()); i++) {
      for (int j = index[i - 1]; j < index[i]; j++) {
        xx[i] += static_cast<long double>(dataset[j].first) *
                 static_cast<long double>(dataset[j].first);
        x[i] += static_cast<long double>(dataset[j].first);
        px[i] += static_cast<long double>(dataset[j].first) *
                 static_cast<long double>(dataset[j].second);
        p[i] += static_cast<long double>(dataset[j].second);
      }
      xx[i] += xx[i - 1];
      x[i] += x[i - 1];
      px[i] += px[i - 1];
      p[i] += p[i - 1];
    }
    xx[index.size() - 1] +=
        static_cast<long double>(dataset[index[index.size() - 1]].first) *
        static_cast<long double>(dataset[index[index.size() - 1]].first);
    x[index.size() - 1] +=
        static_cast<long double>(dataset[index[index.size() - 1]].first);
    px[index.size() - 1] +=
        static_cast<long double>(dataset[index[index.size() - 1]].first) *
        static_cast<long double>(dataset[index[index.size() - 1]].second);
    p[index.size() - 1] +=
        static_cast<long double>(dataset[index[index.size() - 1]].second);

    // store the parameters of each segment
    for (int i = 0; i < index.size() - 1; i++) {
      for (int j = i + 1; j < index.size(); j++) {
        int tmpSize = index[j] - index[i];

        double theta1 = 0.0001, theta2 = 0.666;
        long double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        t1 = xx[j] - xx[i];
        t2 = x[j] - x[i];
        t3 = px[j] - px[i];
        t4 = p[j] - p[i];
        if (t1 * tmpSize - t2 * t2 == 0) {
          if (dataset[index[j]].first - dataset[index[i]].first == 0) {
            theta1 = 0;
            theta2 = dataset[index[j]].second;
          } else {
            theta1 = (static_cast<long double>(dataset[index[j]].second) -
                      static_cast<long double>(dataset[index[i]].second)) /
                     (static_cast<long double>(dataset[index[j]].first) -
                      static_cast<long double>(dataset[index[i]].first));
            theta2 = static_cast<long double>(dataset[index[j]].second) -
                     theta1 * static_cast<long double>(dataset[index[j]].first);
          }
        } else {
          theta1 = (t3 * tmpSize - t2 * t4) / (t1 * tmpSize - t2 * t2);
          theta2 = (t1 * t4 - t2 * t3) / (t1 * tmpSize - t2 * t2);
        }
        if (theta1 <= 0) {
          theta1 = std::abs(theta1);
        }

        theta.insert({{index[i], index[j]}, {theta1, theta2}});
      }
    }
  }

  /**
   * @brief calculate the entropy of each segment
   *
   * @param[in] leftIdx the left index of the sub-dataset
   * @param[in] rightIdx the right-index of the sub-dataset
   * @param[in] y1
   * @param[in] y2
   * @return double: entropy
   */
  double Entropy(int leftIdx, int rightIdx, double y1, double y2) {
    auto tmp_theta = theta.find({leftIdx, rightIdx});
    double a = tmp_theta->second.first;
    double entropy = -DBL_MAX;
    if (a > 0) {
      entropy = log2(a) * (y2 - y1);
    }
    return entropy;
  }

 public:
  //*** Private Data Members of CandidatePLR Objects
  /**
   * @brief params for the corresponding segment, each element is {{the index of
   * the left candidate points in the dataset, the index of the right candidate
   * points in the dataset}, {the slope, the intercept}}
   */
  std::map<std::pair<int, int>, std::pair<double, double>> theta;
};

#endif  // NODES_INNERNODE_CANDIDATE_PLR_H_
