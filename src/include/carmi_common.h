/**
 * @file carmi_common.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-04-01
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CARMI_COMMON_H_
#define SRC_INCLUDE_CARMI_COMMON_H_

#include "./carmi.h"
// #include <float.h>

// #include <map>
// #include <utility>
#include <vector>

// #include "../params.h"

template <typename KeyType, typename ValueType>
class CARMICommon {
 public:
  typedef CARMI<KeyType, ValueType> carmi_impl;
  typedef typename CARMI<KeyType, ValueType>::DataType DataType;
  typedef typename CARMI<KeyType, ValueType>::DataVectorType DataVectorType;

 private:
  carmi_impl carmi_tree;

 public:
  // CARMICommon(const DataVectorType &dataset, int childNum, int kInnerID,
  //             int kLeafID);  // for static structure
  CARMICommon() {}
  CARMICommon(const DataVectorType &dataset, int size, double ratio,
              double rate, int thre = 1024, double insert_left = 0,
              double insert_right = 1);

  class iterator {
   public:
    inline iterator() : tree(NULL), currnode(NULL), currslot(0) {}
    explicit inline iterator(CARMICommon *t)
        : tree(t), currnode(NULL), currslot(-1) {}
    inline iterator(CARMICommon *t, BaseNode *l, int s)
        : tree(t), currnode(l), currslot(s) {}
    inline const KeyType key() const {
      if (currnode == NULL || tree == NULL) {
        return DBL_MIN;
      }
      int left = currnode->array.m_left;
      return tree->carmi_tree.entireData[left + currslot].first;
    }
    inline const ValueType data() const {
      if (currnode == NULL) {
        return DBL_MIN;
      }
      int left = currnode->array.m_left;
      return tree->carmi_tree.entireData[left + currslot].second;
    }
    inline iterator &begin() const {
      static iterator it;
      it.tree = this;
      it.currnode = &tree->carmi_tree.entireChild[tree->carmi_tree.firstLeaf];
      it.currslot = 0;
      return it;
    }
    inline iterator &end() const {
      static iterator it;
      it.tree = this->tree;
      it.currnode = NULL;
      it.currslot = -1;
      return it;
    }
    inline bool operator==(const iterator &x) const {
      return (x.tree == tree && x.currnode == currnode) &&
             (x.currslot == currslot);
    }
    inline bool operator!=(const iterator &x) const {
      return (x.currnode != currnode || x.tree != tree) ||
             (x.currslot != currslot);
    }

    inline iterator &operator++() {
      int left = currnode->array.m_left;
      while (currslot < (currnode->array.flagNumber & 0x00FFFFFF) ||
             currnode->array.nextLeaf != -1) {
        currslot++;
        if (tree->carmi_tree.entireData[left + currslot].first != DBL_MIN) {
          return *this;
        }
        if (currslot == (currnode->array.flagNumber & 0x00FFFFFF))
          currnode = &(tree->carmi_tree.entireChild[currnode->array.nextLeaf]);
      }
      return end();
    }

    CARMICommon *tree;
    BaseNode *currnode;
    int currslot;
  };

  // main functions
 public:
  iterator Find(double key) {
    iterator it(this);
    it.currnode = carmi_tree.Find(key, &it.currslot);
    return it;
  }

  bool Insert(DataType data) { return carmi_tree.Insert(data); }
  bool Update(DataType data) { return carmi_tree.Update(data); }
  bool Delete(double key) { return carmi_tree.Delete(key); }

  int RootType() { return carmi_tree.rootType; }

  long double CalculateSpace() const { return carmi_tree.CalculateSpace(); }

  void PrintStructure(int level, NodeType type, int idx,
                      std::vector<int> *levelVec,
                      std::vector<int> *nodeVec) const {
    carmi_tree.PrintStructure(level, type, idx, levelVec, nodeVec);
  }
};

template <typename KeyType, typename ValueType>
CARMICommon<KeyType, ValueType>::CARMICommon(const DataVectorType &dataset,
                                             int size, double ratio,
                                             double rate, int thre,
                                             double insert_left,
                                             double insert_right) {
  if (ratio == carmi_params::kWriteHeavy) {
    carmi_tree.kIsWriteHeavy = true;
  } else {
    carmi_tree.kIsWriteHeavy = false;
  }
  DataVectorType initDataset(dataset);
  DataVectorType findQuery = initDataset;
  for (int i = 0; i < findQuery.size(); i++) {
    findQuery[i].second = 1;
  }
  DataVectorType insertQuery;
  std::vector<int> insertQueryIndex;

  if (ratio != carmi_params::kWritePartial &&
      ratio != carmi_params::kReadOnly) {
    if (ratio == carmi_params::kRangeScan) {
      ratio = carmi_params::kReadHeavy;
    }
    int cnt = round(1.0 / (1.0 - ratio));
    for (int j = cnt - 1; j < size; j += cnt) {
      insertQuery.push_back({initDataset[j].first, 1});
      insertQueryIndex.push_back(j);
    }
  } else if (ratio == carmi_params::kWritePartial) {
    for (int j = size * insert_left; j < size * insert_right; j += 2) {
      insertQuery.push_back({initDataset[j].first, 1});
      insertQueryIndex.push_back(j);
    }
  }
  carmi_impl tmp(initDataset, findQuery, insertQuery, insertQueryIndex, rate,
                 thre);
  carmi_tree = tmp;
  carmi_tree.InitEntireData(0, initDataset.size(), false);
  carmi_tree.Construction(initDataset, findQuery, insertQuery);
}
#endif  // SRC_INCLUDE_CARMI_COMMON_H_
