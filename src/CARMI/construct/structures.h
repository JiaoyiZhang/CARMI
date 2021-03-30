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
#ifndef SRC_CARMI_CONSTRUCT_STRUCTURES_H_
#define SRC_CARMI_CONSTRUCT_STRUCTURES_H_

#include <iostream>
#include <vector>

struct RootStruct {
  int rootType;
  int rootChildNum;
  RootStruct(int t, int c) {
    rootType = t;
    rootChildNum = c;
  }
};

struct NodeCost {
  double time;
  double space;
  double cost;
};

class IndexPair {
 public:
  int left;
  int size;
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

class DataRange {
 public:
  IndexPair initRange;
  IndexPair findRange;
  IndexPair insertRange;
  DataRange(IndexPair init, IndexPair find, IndexPair insert)
      : initRange(init), findRange(find), insertRange(insert) {}
};

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

enum NodeType {
  LR_ROOT_NODE,
  PLR_ROOT_NODE,
  HIS_ROOT_NODE,
  BS_ROOT_NODE,
  LR_INNER_NODE,
  PLR_INNER_NODE,
  HIS_INNER_NODE,
  BS_INNER_NODE,
  ARRAY_LEAF_NODE,
  GAPPED_ARRAY_LEAF_NODE,
  EXTERNAL_ARRAY_LEAF_NODE
};

#endif  // SRC_CARMI_CONSTRUCT_STRUCTURES_H_
