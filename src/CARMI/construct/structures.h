/**
 * @file structures.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_CONSTRUCT_STRUCTURES_H_
#define SRC_CARMI_CONSTRUCT_STRUCTURES_H_

/**
 * structures for CARMI
 */

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
  bool isInnerNode;
};

class DataRange {
 public:
  int left;
  int size;
  DataRange(int l, int s) {
    left = l;
    size = s;
  }
  DataRange(const DataRange &data) {
    left = data.left;
    size = data.size;
  }

  bool operator<(const DataRange &a) const {
    if (left == a.left)
      return size < a.size;
    else
      return left < a.left;
  }
};

struct MapKey {
  bool isInnerNode;
  DataRange initRange;
  bool operator<(const MapKey &a) const {
    if (initRange.left == a.initRange.left)
      return initRange.size < a.initRange.size;
    else
      return initRange.left < a.initRange.left;
  }
};

struct ParamStruct {
  int type;                   // 0-4
  int childNum;               // for inner nodes
  double density;             // for leaf nodes
  std::vector<MapKey> child;  // the key in the corresponding map
  ParamStruct() {}
  ParamStruct(int t, int c, double d, std::vector<MapKey> tmpChild) {
    type = t;
    childNum = c;
    density = d;
    child = tmpChild;
  }
};

// change the name
class IndexPair {
 public:
  DataRange initRange;
  DataRange findRange;
  DataRange insertRange;
  IndexPair(DataRange init, DataRange find, DataRange insert)
      : initRange(init), findRange(find), insertRange(insert) {}
};

class SubDataset {
 public:
  std::vector<DataRange> subInit;
  std::vector<DataRange> subFind;
  std::vector<DataRange> subInsert;

  explicit SubDataset(int c)
      : subInit(std::vector<DataRange>(c, {-1, 0})),
        subFind(std::vector<DataRange>(c, {-1, 0})),
        subInsert(std::vector<DataRange>(c, {-1, 0})) {}
  ~SubDataset() {}
};

enum NodeTypeID {
  LR_ROOT_NODE = 0,
  PLR_ROOT_NODE = 1,
  HIS_ROOT_NODE = 2,
  BS_ROOT_NODE = 3,
  LR_INNER_NODE = 4,
  PLR_INNER_NODE = 5,
  HIS_INNER_NODE = 6,
  BS_INNER_NODE = 7,
  ARRAY_LEAF_NODE = 8,
  GAPPED_ARRAY_LEAF_NODE = 9,
  EXTERNAL_ARRAY_LEAF_NODE = 10
};
#endif  // SRC_CARMI_CONSTRUCT_STRUCTURES_H_
