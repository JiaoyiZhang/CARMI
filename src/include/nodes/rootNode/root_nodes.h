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
#ifndef SRC_INCLUDE_NODES_ROOTNODE_ROOT_NODES_H_
#define SRC_INCLUDE_NODES_ROOTNODE_ROOT_NODES_H_

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
 * @tparam DataVectorType the type of dataset
 * @tparam KeyType the type of the given key value
 */
template <typename DataVectorType, typename KeyType>
class PLRType {
 public:
  PLRType() = default;
  /**
   * @brief Construct a new PLRType object
   *
   * @param child the child number of the root node
   */
  explicit PLRType(int child) {
    flagNumber = (PLR_ROOT_NODE << 24);
    model.length = child - 1;
  }
  PiecewiseLR<DataVectorType, KeyType> model;  ///< the P. LR root model
  PrefetchPLR<DataVectorType, KeyType>
      fetch_model;  ///< 20 Byte, the prefetch prediction model
  int flagNumber;   ///< type
  int childLeft;    ///< start_index
};
#endif  // SRC_INCLUDE_NODES_ROOTNODE_ROOT_NODES_H_
