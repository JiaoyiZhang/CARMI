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
 * @brief designed for piecewise linear regression model. This structure records
 * all the contents that need to be stored in the training process of piecewise
 * linear function, which is the item in the dp table.
 */
struct SegmentPoint {
  /**
   * @brief the current cost
   */
  double cost;

  /**
   * @brief the key values
   */
  double key[12] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

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
 * the line segment between two points, entropy, etc. in the process of dynamic
 * programming algorithm in P. LR model, which is used to assist and  accelerate
 * the dp algorithm.
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
    std::vector<double> xx(index.size(), 0);
    std::vector<double> x(index.size(), 0);
    std::vector<double> px(index.size(), 0);
    std::vector<double> p(index.size(), 0);
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

    // store the parameters of each segment
    for (int i = 0; i < index.size() - 1; i++) {
      for (int j = i + 1; j < index.size(); j++) {
        int tmpSize = index[j] - index[i];

        double theta1 = 0.0001, theta2 = 0.666;
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        t1 = xx[j] - xx[i];
        t2 = x[j] - x[i];
        t3 = px[j] - px[i];
        t4 = p[j] - p[i];

        theta1 = (t3 * tmpSize - t2 * t4) / (t1 * tmpSize - t2 * t2);
        theta2 = (t1 * t4 - t2 * t3) / (t1 * tmpSize - t2 * t2);

        theta.insert({{index[i], index[j]}, {theta1, theta2}});
      }
    }
  }

  /**
   * @brief calculate the entropy of each segment
   *
   * @param[in] leftIdx the left index of the sub-dataset
   * @param[in] rightIdx the right-index of the sub-dataset
   * @return double: entropy
   */
  double Entropy(int leftIdx, int rightIdx) {
    auto tmp_theta = theta.find({leftIdx, rightIdx});
    double a = tmp_theta->second.first;
    double entropy = log2(a) * (rightIdx - leftIdx);
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
