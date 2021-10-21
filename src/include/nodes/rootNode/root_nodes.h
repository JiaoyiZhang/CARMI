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
 * The piecewise linear regression model with five segments to allocate data
 * points more evenly.
 *
 * @tparam DataVectorType the type of dataset
 * @tparam KeyType the type of the given key value
 */
template <typename DataVectorType, typename KeyType>
class PLRType {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief The type of the model
   */
  typedef PiecewiseLR<DataVectorType, KeyType> ModelType;

  /**
   * @brief Construct a new PLRType object
   *
   */
  PLRType() = default;

  /**
   * @brief Construct a new PLRType object
   *
   * @param child[in] the child number of the root node
   */
  explicit PLRType(int child) {
    flagNumber = PLR_ROOT_NODE;
    model.length = child - 1;
  }

  PLRType(int childNumber, const DataVectorType &dataset) {
    flagNumber = PLR_ROOT_NODE;
    model.length = childNumber - 1;
    model.Train(dataset);
  }

 public:
  // *** Static Constant Options and Values of PLR Root Node Objects

  /**
   * @brief The time cost of the plr root node.
   */
  static constexpr int kTimeCost = carmi_params::kPLRRootTime;

 public:
  //*** Public Data Members of P. LR Root Node Objects

  /**
   * @brief the main root model: piecewise linear regression model with five
   * segments to allocate the dataset to the child nodes.
   *
   * We use this model to predict the index of the next node, and use the raw
   * output of this model (leaf index before rounding down) as the input to the
   * prefetch prediction model.
   */
  PiecewiseLR<DataVectorType, KeyType> model;

  /**
   * @brief the prefetch prediction model.
   *
   * This model is also a piecewise linear regression model, which uses the
   * output of the main root model to compute a block index. In this model,
   * the slope and intercept of each segment are forced to be integers, so that
   * within each segment, each leaf node is mapped to the same number of data
   * blocks.
   */
  PrefetchPLR<DataVectorType, KeyType> fetch_model;

  /**
   * @brief the type of the root node: PLR_ROOT_NODE
   */
  int flagNumber;
};

#endif  // NODES_ROOTNODE_ROOT_NODES_H_
