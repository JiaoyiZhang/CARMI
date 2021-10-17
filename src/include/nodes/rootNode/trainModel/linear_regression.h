/**
 * @file linear_regression.h
 * @author Jiaoyi
 * @brief linear regression model
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_NODES_ROOTNODE_TRAINMODEL_LINEAR_REGRESSION_H_
#define SRC_INCLUDE_NODES_ROOTNODE_TRAINMODEL_LINEAR_REGRESSION_H_

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "../../../params.h"

/**
 * @brief linear regression model for root node
 *
 * @tparam DataVectorType the vector type of the dataset
 * @tparam KeyType the type of the key value
 */
template <typename DataVectorType, typename KeyType>
class LinearRegression {
 public:
  LinearRegression() {
    theta1 = 0.0001;
    theta2 = 0.666;
  }
  /**
   * @brief train the lr model
   *
   * @param dataset the original dataset
   */
  void Train(const DataVectorType &dataset) {
    int idx = 0;
    int size = dataset.size();
    std::vector<double> index(size, 0);
    for (int i = 0; i < size; i++) {
      index[idx++] = static_cast<double>(i) / size * length;
    }
    if (size == 0) return;

    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = 0; i < size; i++) {
      t1 += dataset[i].first * dataset[i].first;
      t2 += dataset[i].first;
      t3 += dataset[i].first * index[i];
      t4 += index[i];
    }
    theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
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
  inline double PredictIdx(KeyType key) const {
    // return the predicted idx in the children
    double p = theta1 * key + theta2;
    if (p < 0)
      p = 0;
    else if (p > length)
      p = length;
    return p;
  }

  int length;  ///< the number of child nodes

 private:
  double theta1;  ///< the slope
  double theta2;  ///< the intercept
};
#endif  // SRC_INCLUDE_NODES_ROOTNODE_TRAINMODEL_LINEAR_REGRESSION_H_
