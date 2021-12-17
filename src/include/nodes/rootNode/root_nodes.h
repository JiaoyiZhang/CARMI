/**
 * @file root_nodes.h
 * @author Jiaoyi
 * @brief the details of root nodes
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef NODES_ROOTNODE_ROOT_NODES_H_
#define NODES_ROOTNODE_ROOT_NODES_H_

#include <algorithm>
#include <fstream>
#include <vector>

#include "../../construct/structures.h"
#include "../../params.h"
#include "trainModel/linear_regression.h"
#include "trainModel/piecewiseLR.h"
#include "trainModel/prefetch_plr.h"

/**
 * @brief piecewise linear regression root node
 *
 * The piecewise linear regression model with five segments can allocate data
 * points more evenly.
 *
 * Since the root node is always in the cache, we do not limit its size here. We
 * use a five-segment P. LR model, occupying 76 bytes. In addition, to support
 * the prefetch function, we add a prefetch prediction model to speed up the
 * process of accessing a data point.
 *
 * @tparam DataVectorType the type of dataset
 * @tparam KeyType the type of the given key value
 */
template <typename DataVectorType, typename KeyType>
class PLRType {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief The type of the model: piecewise linear regression
   */
  typedef PiecewiseLR<DataVectorType, KeyType> ModelType;

  /**
   * @brief Construct a new PLRType object with the default constructor
   */
  PLRType() = default;

  /**
   * @brief Construct a new PLRType object and train the plr model with the
   * given dataset.
   *
   * PLR root node uses a piecewise linear regression model to predict the index
   * of the next node. When finding the position of the data point, we first
   * find the first breakpoint greater than or equal to the given key value, and
   * then use the corresponding model parameters for the calculation and
   * boundary processing.
   *
   * @param[in] childNum the number of the child nodes in the root node
   * @param[in] dataset the dataset used to train the plr model of the root node
   */
  PLRType(int childNum, const DataVectorType &dataset) {
    flagNumber = PLR_ROOT_NODE;
    model.maxChildIdx = std::max(2, childNum - 1);
    model.Train(dataset);
  }

 public:
  // *** Static Constant Options and Values of P. LR Root Node Objects

  /**
   * @brief The time cost of the plr root node.
   */
  static constexpr double kTimeCost = carmi_params::kPLRRootTime;

 public:
  //*** Public Data Members of P. LR Root Node Objects

  /**
   * @brief the main root model: piecewise linear regression model with five
   * segments to allocate the dataset to the child nodes.
   *
   * We use this model to predict the index of the next node, and use the raw
   * output of this model (leaf index before rounding down) as the input to the
   * prefetch prediction model. (72 bytes)
   */
  ModelType model;

  /**
   * @brief the prefetch prediction model.
   *
   * This model is also a piecewise linear regression model, which uses the
   * output of the main root model to compute a block index. In this model,
   * the slope and intercept of each segment are forced to be integers, so that
   * within each segment, each leaf node is mapped to the same number of data
   * blocks.
   */
  PrefetchPLR fetch_model;

  /**
   * @brief the type of the root node: PLR_ROOT_NODE (4 bytes)
   */
  int flagNumber;
};

#endif  // NODES_ROOTNODE_ROOT_NODES_H_
