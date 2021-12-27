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
#ifndef NODES_ROOTNODE_TRAINMODEL_LINEAR_REGRESSION_H_
#define NODES_ROOTNODE_TRAINMODEL_LINEAR_REGRESSION_H_

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
 * This model is a very simple root node consisting of a linear regression
 * model. In addition to the number of child nodes, only two linear regression
 * model parameters need to be stored. We only need one model prediction and the
 * boundary condition processing to get the index of the next child node.
 *
 * @tparam DataVectorType the vector type of the dataset
 * @tparam KeyType the type of the key value
 */
template <typename DataVectorType, typename KeyType>
class LinearRegression {
 public:
  // *** Constructor

  /**
   * @brief Construct a new Linear Regression object and set the default value
   * of the linear regression model parameters.
   */
  LinearRegression() {
    slope = 0.0001;
    intercept = 0.666;
    maxChildIdx = 2;
    minValue = 0;
  }

  /**
   * @brief use the given dataset to train the lr model
   *
   * @param[in] dataset the original dataset, each data point is: {key, value}
   */
  void Train(const DataVectorType &dataset) {
    int idx = 0;
    int size = dataset.size();
    if (size == 0) return;
    minValue = dataset[0].first;
    std::vector<double> index(size, 0);
    // construct the training dataset, x is the key value in the dataset, y is
    // the corresponding ratio of index in the maxChildIdx
    for (int i = 0; i < size; i++) {
      index[idx++] = static_cast<double>(i) / size * maxChildIdx;
    }

    // train the lr model
    long double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = 0; i < size; i++) {
      t1 += static_cast<long double>(dataset[i].first - minValue) *
            static_cast<long double>(dataset[i].first - minValue);
      t2 += static_cast<long double>(dataset[i].first - minValue);
      t3 += static_cast<long double>(dataset[i].first - minValue) *
            static_cast<long double>(index[i]);
      t4 += static_cast<long double>(index[i]);
    }
    if (t1 * size - t2 * t2) {
      slope = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
      intercept = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
    } else {
      slope = 1.0;
      intercept = 1.0;
    }
  }

  /**
   * @brief output the unrounded index of the next node of the given key value
   *
   * @param[in] key the given key value
   * @return double: the unrounded index
   */
  inline double Predict(KeyType key) const {
    // predict the index of the next node using the lr model
    double p = slope * static_cast<double>(key - minValue) + intercept;
    // boundary processing
    if (p < 0)
      p = 0;
    else if (p > maxChildIdx)
      p = maxChildIdx;
    return p;
  }

 public:
  //*** Public Data Members of LR Model Objects

  /**
   * @brief The number of the child nodes
   */
  int maxChildIdx;

 private:
  //*** Private Data Members of LR Model Objects

  /**
   * @brief The linear regression parameter: the slope
   */
  double slope;

  /**
   * @brief The linear regression parameter: the intercept
   */
  double intercept;

  /**
   * @brief The minimum value.
   */
  KeyType minValue;
};
#endif  // NODES_ROOTNODE_TRAINMODEL_LINEAR_REGRESSION_H_
