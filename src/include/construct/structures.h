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
 * @brief Root node settings: the type of the root node and the number of child
 * nodes
 */
struct RootStruct {
  /**
   * @brief the type identifier of the root node
   */
  int rootType;

  /**
   * @brief the number of its child nodes
   */
  int rootChildNum;

  /**
   * @brief Construct a new Root Struct object and set the values
   *
   * @param[in] type the type of the root node
   * @param[in] c the number of child nodes
   */
  RootStruct(int type, int c) {
    rootType = type;
    rootChildNum = c;
  }
};

/**
 * @brief three parts of the cost: time cost, space cost and the total cost.
 *
 * The total cost  = time cost + lambda * space cost.
 */
struct NodeCost {
  /**
   * @brief the time cost
   */
  double time;

  /**
   * @brief the space cost
   */
  double space;

  /**
   * @brief the total cost: time cost + lambda * space cost.
   */
  double cost;
};

/**
 * @brief the index range of data points: [left, left + size)
 */
struct IndexPair {
  /**
   * @brief the left index of data points in the dataset
   */
  int left;

  /**
   * @brief the size of data points
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
 * @brief the index ranges of sub-initDataset, sub-findQuery and
 * sub-insertQuery: {initDataset: {left, size}, findQuery: {left, size},
 * insertQuery: {left, size}}
 */
class DataRange {
 public:
  /**
   * @brief the index range of initDataset: {the left index of the sub-dataset
   * in the initDataset, the size of the sub-dataset}
   */
  IndexPair initRange;

  /**
   * @brief the index range of findQuery: {the left index of the sub-dataset
   * in the findQuery, the size of the sub-dataset}
   */
  IndexPair findRange;

  /**
   * @brief the index range of insertQuery: {the left index of the sub-dataset
   * in the insertQuery, the size of the sub-dataset}
   */
  IndexPair insertRange;

  /**
   * @brief Construct a new Data Range object
   *
   * @param[in] init the index range of sub-initDataset: {the left index of the
   * sub-dataset in the initDataset, the size of the sub-dataset}
   * @param[in] find the index range of sub-findQuery: {the left index of the
   * sub-dataset in the findQuery, the size of the sub-dataset}
   * @param[in] insert the index range of sub-insertQuery: {the left index of
   * the sub-dataset in the insertQuery, the size of the sub-dataset}
   */
  DataRange(IndexPair init, IndexPair find, IndexPair insert)
      : initRange(init), findRange(find), insertRange(insert) {}
};

/**
 * @brief the starting index and size of sub-dataset in each child node, each
 * element is: {the vector of the sub-initDataset, the vector of the
 * sub-findDataset, the vector of sub-insertDataset}. Each sub-dataset is
 * represented by: {left, size}, which means the range of it in the dataset is
 * [left, left + size).
 */
class SubDataset {
 public:
  /**
   * @brief the IndexPair vector of sub-initDataset, each element is: {the left
   * index of the sub-dataset in the initDataset, the size of the sub-dataset}
   */
  std::vector<IndexPair> subInit;

  /**
   * @brief the IndexPair vector of sub-findDataset, each element is: {the left
   * index of the sub-dataset in the findDataset, the size of the sub-dataset}
   */
  std::vector<IndexPair> subFind;

  /**
   * @brief the IndexPair vector of sub-insertDataset, each element is: {the
   * left index of the sub-dataset in the insertDataset, the size of the
   * sub-dataset}
   */
  std::vector<IndexPair> subInsert;

  /**
   * @brief Construct a new SubDataset object and the size of the vector is c
   *
   * @param[in] c the size of the vector
   */
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
   * @brief the structure of a data block which actually stores the data points,
   * its size is determined by the kMaxLeafNodeSize and the type of the data
   * point. Each element in slots is: {key value, value}.
   */
  std::pair<KeyType, ValueType> slots[carmi_params::kMaxLeafNodeSize /
                                      sizeof(std::pair<KeyType, ValueType>)];

  /**
   * @brief Construct a new Leaf Slots object and set the default value of each
   * element to the pair of {DBL_MAX, DBL_MAX}
   */
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
        this->slots[i] = currnode.slots[i];
      }
    }
    return *this;
  }
};
#endif  // CONSTRUCT_STRUCTURES_H_
