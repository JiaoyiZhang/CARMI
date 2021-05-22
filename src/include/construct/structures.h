/**
 * @file structures.h
 * @author Jiaoyi
 * @brief structures for CARMI
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_STRUCTURES_H_
#define SRC_INCLUDE_CONSTRUCT_STRUCTURES_H_

#include <iostream>
#include <vector>

/**
 * @brief the structure of the information
 * of the root node
 *
 */
struct RootStruct {
  int rootType;      // the type of the root node
  int rootChildNum;  // the number of the root node's children
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
  double time;   // the time cost of tree
  double space;  // the space cost of tree
  double cost;   // the total cost of tree
};

/**
 * @brief the index range of data points
 *
 */
class IndexPair {
 public:
  int left;  // the start index of data points
  int size;  // the size of data points
  IndexPair() { left = -1, size = 0; }
  IndexPair(int l, int s) {
    left = l;
    size = s;
  }
  IndexPair(const IndexPair &data) {
    left = data.left;
    size = data.size;
  }

  bool operator<(const IndexPair &a) const {
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
  IndexPair initRange;    // the bound range of initDataset
  IndexPair findRange;    // the bound range of findQuery
  IndexPair insertRange;  // the bound range of insertQuery
  DataRange(IndexPair init, IndexPair find, IndexPair insert)
      : initRange(init), findRange(find), insertRange(insert) {}
};

/**
 * @brief the range of subdatasets
 *
 */
class SubDataset {
 public:
  std::vector<IndexPair> subInit;
  std::vector<IndexPair> subFind;
  std::vector<IndexPair> subInsert;

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
  LR_ROOT_NODE,
  LR_INNER_NODE,
  PLR_INNER_NODE,
  HIS_INNER_NODE,
  BS_INNER_NODE,
  ARRAY_LEAF_NODE,
  GAPPED_ARRAY_LEAF_NODE,
  EXTERNAL_ARRAY_LEAF_NODE
};

#endif  // SRC_INCLUDE_CONSTRUCT_STRUCTURES_H_
