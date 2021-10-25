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
#ifndef CONSTRUCT_STRUCTURES_H_
#define CONSTRUCT_STRUCTURES_H_

#include <float.h>

#include <iostream>
#include <utility>
#include <vector>

#include "../params.h"

/**
 * @brief Root node settings: the type of root node and the number of child
 * nodes
 */
struct RootStruct {
  /**
   * @brief the type of the root node
   */
  int rootType;

  /**
   * @brief the number of the root node's children
   *
   */
  int rootChildNum;

  /**
   * @brief Construct a new Root Struct object
   *
   * @param[in] t the root type
   * @param[in] c the child number
   */
  RootStruct(int t, int c) {
    rootType = t;
    rootChildNum = c;
  }
};

/**
 * @brief the structure of cost model: time cost, space cost and the total cost:
 * time + lambda * space
 */
struct NodeCost {
  /**
   * @brief the time cost
   *
   */
  double time;

  /**
   * @brief the space cost
   *
   */
  double space;

  /**
   * @brief the total cost
   *
   */
  double cost;
};

/**
 * @brief the index range of data points: [left, left + size)
 */
struct IndexPair {
  /**
   * @brief the start index of data points
   *
   */
  int left;

  /**
   * @brief the size of data points
   *
   */
  int size;

  bool operator<(const IndexPair& a) const {
    if (left == a.left)
      return size < a.size;
    else
      return left < a.left;
  }
};

/**
 * @brief the index ranges of initDataset, findQuery and insertQuery
 */
class DataRange {
 public:
  /**
   * @brief the index range of initDataset
   *
   */
  IndexPair initRange;

  /**
   * @brief the index range of findQuery
   *
   */
  IndexPair findRange;

  /**
   * @brief the index range of insertQuery
   *
   */
  IndexPair insertRange;

  DataRange(IndexPair init, IndexPair find, IndexPair insert)
      : initRange(init), findRange(find), insertRange(insert) {}
};

/**
 * @brief the range of each sub-datasets corresponding to the child nodes of the
 * current nodes
 */
class SubDataset {
 public:
  /**
   * @brief the IndexPair vector of sub-initDataset
   */
  std::vector<IndexPair> subInit;

  /**
   * @brief the IndexPair vector of sub-findDataset
   */
  std::vector<IndexPair> subFind;

  /**
   * @brief the IndexPair vector of sub-insertDataset
   */
  std::vector<IndexPair> subInsert;

  explicit SubDataset(int c)
      : subInit(std::vector<IndexPair>(c, {-1, 0})),
        subFind(std::vector<IndexPair>(c, {-1, 0})),
        subInsert(std::vector<IndexPair>(c, {-1, 0})) {}
  ~SubDataset() {}
};

/**
 * @brief enumerate type of all node types
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
 * @brief the structure of a data block
 *
 * This structure is designed for the CF array leaf nodes, so as to make better
 * use of the cache mechanism to speed up data access. The size of this class is
 * fixed as kMaxLeafNodeSize.
 *
 * @tparam KeyType the type of the given key value
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class LeafSlots {
 public:
  /**
   * @brief store the data points
   *
   */
  std::pair<KeyType, ValueType> slots[carmi_params::kMaxLeafNodeSize /
                                      sizeof(std::pair<KeyType, ValueType>)];

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
#endif  // CONSTRUCT_STRUCTURES_H_
