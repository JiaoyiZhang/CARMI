/**
 * @file structures.h
 * @author Jiaoyi
 * @brief structures for CARMI
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_STRUCTURES_H_
#define SRC_INCLUDE_CONSTRUCT_STRUCTURES_H_

#include <float.h>

#include <iostream>
#include <utility>
#include <vector>

#include "../params.h"

/**
 * @brief the structure of the information
 * of the root node
 *
 */
struct RootStruct {
  int rootType;      ///< the type of the root node
  int rootChildNum;  ///< the number of the root node's children
  /**
   * @brief Construct a new Root Struct object
   *
   * @param t the root type
   * @param c the child number
   */
  RootStruct(int t, int c) {
    rootType = t;
    rootChildNum = c;
  }
};

/**
 * @brief the structure of cost model
 *
 */
struct NodeCost {
  double time;   ///< the time cost of tree
  double space;  ///< the space cost of tree
  double cost;   ///< the total cost of tree
};

/**
 * @brief the index range of data points
 *
 */
struct IndexPair {
  int left;  ///< the start index of data points
  int size;  ///< the size of data points

  bool operator<(const IndexPair& a) const {
    if (left == a.left)
      return size < a.size;
    else
      return left < a.left;
  }
};

/**
 * @brief the bound ranges of initDataset, findQuery and insertQuery
 *
 */
class DataRange {
 public:
  IndexPair initRange;    ///< the bound range of initDataset
  IndexPair findRange;    ///< the bound range of findQuery
  IndexPair insertRange;  ///< the bound range of insertQuery
  DataRange(IndexPair init, IndexPair find, IndexPair insert)
      : initRange(init), findRange(find), insertRange(insert) {}
};

/**
 * @brief the range of subdatasets
 *
 */
class SubDataset {
 public:
  std::vector<IndexPair> subInit;  ///< the IndexPair vector of sub-initDataset
  std::vector<IndexPair> subFind;  ///< the IndexPair vector of sub-findDataset
  std::vector<IndexPair>
      subInsert;  ///< the IndexPair vector of sub-insertDataset

  explicit SubDataset(int c)
      : subInit(std::vector<IndexPair>(c, {-1, 0})),
        subFind(std::vector<IndexPair>(c, {-1, 0})),
        subInsert(std::vector<IndexPair>(c, {-1, 0})) {}
  ~SubDataset() {}
};

/**
 * @brief enumerate type of all node types
 *
 */
enum NodeType {
  PLR_ROOT_NODE,
  LR_INNER_NODE,
  PLR_INNER_NODE,
  HIS_INNER_NODE,
  BS_INNER_NODE,
  ARRAY_LEAF_NODE,
  EXTERNAL_ARRAY_LEAF_NODE
};

/**
 * @brief a data block
 *
 * @tparam KeyType the type of the given key
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class LeafSlots {
 public:
  std::pair<KeyType, ValueType>
      slots[carmi_params::kMaxLeafNodeSize /
            sizeof(
                std::pair<KeyType, ValueType>)];  ///< used to store data points

  LeafSlots() {
    int len =
        carmi_params::kMaxLeafNodeSize / sizeof(std::pair<KeyType, ValueType>);
    for (int i = 0; i < len; i++) {
      slots[i] = {DBL_MAX, DBL_MAX};
    }
  }

  LeafSlots& operator=(const LeafSlots& currnode) {
    if (this != &currnode) {
      int len = carmi_params::kMaxLeafNodeSize /
                sizeof(std::pair<KeyType, ValueType>);
      for (int i = 0; i < len; i++) {
        this->slots[i].first = currnode.slots[i].first;
        this->slots[i].second = currnode.slots[i].second;
      }
    }
    return *this;
  }
};
#endif  // SRC_INCLUDE_CONSTRUCT_STRUCTURES_H_
