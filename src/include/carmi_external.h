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
#ifndef SRC_INCLUDE_CARMI_EXTERNAL_H_
#define SRC_INCLUDE_CARMI_EXTERNAL_H_

#include "./carmi.h"
// #include <float.h>

// #include <map>
#include <utility>
#include <vector>

// #include "../params.h"

template <typename KeyType>
class CARMIExternal {
 public:
  typedef CARMI<KeyType, KeyType> carmi_impl;
  typedef typename CARMI<KeyType, KeyType>::DataType DataType;
  typedef typename CARMI<KeyType, KeyType>::DataVectorType DataVectorType;
  typedef std::pair<KeyType, std::vector<KeyType>> ActualDataType;

 private:
  carmi_impl carmi_tree;

 public:
  //   CARMIExternal(const DataVectorType &dataset, int childNum, int
  //   kInnerID,
  //               int kLeafID);  // for static structure
  CARMIExternal() {}
  CARMIExternal(KeyType *dataset, int dataset_size, int max_size,
                int record_size, double rate, int thre = 1024);

  class iterator {
   public:
    inline iterator() : tree(NULL), currnode(NULL), currslot(0) {}
    explicit inline iterator(CARMIExternal *t)
        : tree(t), currnode(NULL), currslot(-1) {}
    inline iterator(CARMIExternal *t, BaseNode *l, int s)
        : tree(t), currnode(l), currslot(s) {}
    inline const KeyType key() const {
      int left = currnode->externalArray.m_left;
      if (tree == NULL || left + currslot > tree->carmi_tree.nowDataSize) {
        return DBL_MIN;
      }
      return *(tree->carmi_tree.external_data +
               (left + currslot) * tree->carmi_tree.kRecordLen);
    }
    inline const std::vector<KeyType> data() const {
      std::vector<KeyType> res;
      int left = currnode->externalArray.m_left;
      if (tree == NULL || left + currslot > tree->carmi_tree.nowDataSize) {
        return res;
      }
      int len = tree->carmi_tree.kRecordLen;
      for (int i = 1; i < len; i++) {
        res.push_back(
            *(tree->carmi_tree.external_data + (left + currslot) * len + i));
      }
      return res;
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
      return (x.tree != tree || x.currnode != currnode) ||
             (x.currslot != currslot);
    }

    inline iterator &operator++() {
      int len = tree->carmi_tree.kRecordLen;
      int left = currnode->externalArray.m_left;
      while (currslot + left < tree->carmi_tree.nowDataSize) {
        currslot++;
        if (*(tree->carmi_tree.external_data + (left + currslot) * len) !=
            DBL_MIN) {
          return *this;
        }
      }
      return end();
    }

    CARMIExternal *tree;
    BaseNode *currnode;
    int currslot;  // index in external_data
  };

  // main functions
 public:
  iterator Find(KeyType key) {
    iterator it(this);
    it.currnode = carmi_tree.Find(key, &it.currslot);
    return it;
  }

  bool Insert(ActualDataType data) {
    bool isSuccess = carmi_tree.Insert({data.first, data.first});
    if (isSuccess) {
      std::vector<KeyType> res = data.second;
      KeyType *start =
          carmi_tree.external_data + carmi_tree.curr * carmi_tree.kRecordLen;
      *start = data.first;
      for (int i = 1; i < carmi_tree.kRecordLen; i++) {
        *(start + i) = res[i - 1];
      }
    }
    carmi_tree.curr++;
    return isSuccess;
  }
  bool Update(DataType data) { return carmi_tree.Update(data); }
  bool Delete(KeyType key) { return carmi_tree.Delete(key); }

  int RootType() { return carmi_tree.rootType; }

  long double CalculateSpace() const { return carmi_tree.CalculateSpace(); }

  void PrintStructure(int level, NodeType type, int idx,
                      std::vector<int> *levelVec,
                      std::vector<int> *nodeVec) const {
    carmi_tree.PrintStructure(level, type, idx, levelVec, nodeVec);
  }
};

template <typename KeyType>
CARMIExternal<KeyType>::CARMIExternal(KeyType *dataset, int dataset_size,
                                      int max_size, int record_size,
                                      double rate, int thre) {
  DataVectorType initDataset(dataset_size, DataType());
  DataVectorType insertQuery;
  std::vector<int> insertQueryIndex;

  KeyType *idx = dataset;
  int len = record_size / sizeof(KeyType);
  for (int i = 0; i < dataset_size; i++) {
    initDataset[i] = {*idx, 1};
    idx += len;
  }
  DataVectorType findQuery = initDataset;

  for (int i = carmi_params::kExternalInsertLeft; i < dataset_size; i++) {
    insertQuery.push_back({initDataset[i].first, 1});
    insertQueryIndex.push_back(i);
  }

  carmi_impl tmp(initDataset, findQuery, insertQuery, insertQueryIndex, rate,
                 thre);
  carmi_tree = tmp;

  carmi_tree.external_data = dataset;
  carmi_tree.kRecordLen = record_size / sizeof(KeyType);
  carmi_tree.nowDataSize = dataset_size;
  carmi_tree.entireDataSize = max_size;
  carmi_tree.curr = carmi_params::kExternalInsertLeft;

  carmi_tree.Construction(initDataset, findQuery, insertQuery);
}
#endif  // SRC_INCLUDE_CARMI_EXTERNAL_H_
