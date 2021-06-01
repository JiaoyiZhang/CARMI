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

#include <vector>

#include "./carmi.h"
#include "construct/construction.h"
#include "dataManager/child_array.h"
#include "dataManager/datapoint.h"
#include "func/delete_function.h"
#include "func/find_function.h"
#include "func/insert_function.h"
#include "func/print_structure.h"
#include "func/update_function.h"

template <typename KeyType, typename ValueType>
class CARMICommon {
 public:
  typedef CARMI<KeyType, ValueType> carmi_impl;
  typedef typename CARMI<KeyType, ValueType>::DataType DataType;
  typedef typename CARMI<KeyType, ValueType>::DataVectorType DataVectorType;

 private:
  carmi_impl carmi_tree;

 public:
  /**
   * @brief Construct a new CARMICommon object with the range [first,last)
   *
   * @tparam InputIterator the iterator type of input
   * @param first the first input iterator
   * @param last the last input iterator
   * @param ratio readQuerySize/(readQuerySize+writeQuerySize)
   * @param lambda lambda (time + lambda * space)
   */
  template <typename InputIterator>
  CARMICommon(const InputIterator &first, const InputIterator &last,
              double ratio, double lambda) {
    DataVectorType initDataset;
    PreprocessInput<InputIterator>(first, last, &initDataset);
    Init(initDataset, ratio, 0, 1, lambda);
  }

  /**
   * @brief Construct a new CARMICommon object for the situation that insert
   * operations are between left to right with the range [first,last)
   *
   * @tparam InputIterator the iterator type of input
   * @param first the first input iterator
   * @param last the last input iterator
   * @param ratio readQuerySize/(readQuerySize+writeQuerySize)
   * @param lambda lambda (time + lambda * space)
   * @param insert_left the left boundary of the insert operation, the value is
   * between 0 and 1 (left_index / initDataset.size())
   * @param insert_right the right boundary of the insert operation, the value
   * is between 0 and 1 (right_index / initDataset.size())
   */
  template <typename InputIterator>
  CARMICommon(const InputIterator &first, const InputIterator &last,
              double ratio, double lambda, double insert_left,
              double insert_right) {
    DataVectorType initDataset;
    PreprocessInput<InputIterator>(first, last, &initDataset);
    Init(initDataset, ratio, insert_left, insert_right, lambda);
  }

  /**
   * @brief the iterator of CARMICommon
   *
   */
  class iterator {
   public:
    /**
     * @brief Construct a new iterator object
     *
     */
    inline iterator() : tree(NULL), currnode(NULL), currslot(0), currunion(0) {}

    /**
     * @brief Construct a new iterator object
     *
     * @param t the carmi tree
     */
    explicit inline iterator(CARMICommon *t)
        : tree(t), currnode(NULL), currslot(-1), currunion(0) {}

    /**
     * @brief Construct a new iterator object
     *
     * @param t the carmi tree
     * @param l the current node
     * @param s the current slot
     */
    inline iterator(CARMICommon *t, BaseNode<KeyType> *l, int u, int s)
        : tree(t), currnode(l), currunion(u), currslot(s) {}

    /**
     * @brief get the key of this iterator
     *
     * @return const KeyType
     */
    inline const KeyType key() const {
      if (currnode == NULL || tree == NULL || currslot == -1 ||
          currunion >= tree->carmi_tree.kMaxLeafNum) {
        return DBL_MIN;
      }
      int left = currnode->array.m_left;
      if (left + currunion >= tree->carmi_tree.nowDataSize) {
        std::cout << "index is: " << left + currunion << std::endl;
        std::cout << "the index is out of range, size:"
                  << tree->carmi_tree.nowDataSize << std::endl;
      }
      return tree->carmi_tree.entireData[left + currunion]
          .slots[currslot]
          .first;
    }

    /**
     * @brief get the data of this iterator
     *
     * @return const ValueType
     */
    inline const ValueType data() const {
      if (currnode == NULL || tree == NULL || currslot == -1 ||
          currunion >= tree->carmi_tree.kMaxLeafNum) {
        return DBL_MIN;
      }
      int left = currnode->array.m_left;
      if (left + currunion >= tree->carmi_tree.nowDataSize) {
        std::cout << "index is: " << left + currunion << std::endl;
        std::cout << "the index is out of range, size:"
                  << tree->carmi_tree.nowDataSize << std::endl;
      }
      return tree->carmi_tree.entireData[left + currunion]
          .slots[currslot]
          .second;
    }

    /**
     * @brief check if the given x is equal to this iterator
     *
     * @param x
     * @return true
     * @return false
     */
    inline bool operator==(const iterator &x) const {
      return (x.tree == tree && x.currnode == currnode) &&
             (x.currslot == currslot && x.currunion == currunion);
    }

    /**
     * @brief check if the given x is equal to this iterator
     *
     * @param x
     * @return true
     * @return false
     */
    inline bool operator!=(const iterator &x) const {
      return (x.currnode != currnode || x.tree != tree) ||
             (x.currslot != currslot || x.currunion != currunion);
    }

    /**
     * @brief get the iterator of the next slot
     *
     * @return iterator&
     */
    inline iterator &operator++() {
      int left = currnode->array.m_left;
      currslot++;
      while (currunion < (currnode->array.flagNumber & 0x00FFFFFF) ||
             currnode->array.nextLeaf != -1) {
        if (currunion == (currnode->array.flagNumber & 0x00FFFFFF)) {
          currnode = &(tree->carmi_tree.entireChild[currnode->array.nextLeaf]);
          currslot = 0;
          currunion = 0;
          left = currnode->array.m_left;
          continue;
        }
        if (currslot == tree->carmi_tree.kMaxSlotNum ||
            tree->carmi_tree.entireData[left + currunion]
                    .slots[currslot]
                    .first == DBL_MIN) {
          currslot = 0;
          currunion++;
        }
        if (tree->carmi_tree.entireData[left + currunion]
                .slots[currslot]
                .first != DBL_MIN) {
          return *this;
        }
      }
      *this = this->tree->end();
      return *this;
    }

    CARMICommon *tree;            // the carmi tree
    BaseNode<KeyType> *currnode;  // the current leaf node
    int currunion;                // index in unions of this leaf node
    int currslot;                 // index in external_data
  };

 private:
  /**
   * @brief preprocess the input between the first iterator and the last
   * iterator
   *
   * @tparam InputIterator the iterator type of input
   * @param first the first input iterator
   * @param last the last input iterator
   */
  template <typename InputIterator>
  void PreprocessInput(const InputIterator &first, const InputIterator &last,
                       DataVectorType *initDataset);

  /**
   * @brief split the dataset into read queries and write queris according to
   * the ratio, the range of write queries is [insert_leaf*dataset.size(),
   * insert_right*dataset.size())
   *
   * @param initDataset
   * @param ratio readQuerySize/(readQuerySize+writeQuerySize)
   * @param insert_left the left boundary of the insert operation, the value is
   * between 0 and 1 (left_index / initDataset.size())
   * @param insert_right the right boundary of the insert operation, the value
   * is between 0 and 1 (right_index / initDataset.size())
   * @param lambda lambda (time + lambda * space)
   */
  void Init(DataVectorType initDataset, double ratio, double insert_left,
            double insert_right, double lambda);

  // main functions
 public:
  /**
   * @brief find the corresponding iterator of the given key
   *
   * @param key
   * @return iterator
   */
  iterator Find(KeyType key) {
    iterator it(this);
    it.currnode = carmi_tree.Find(key, &it.currunion, &it.currslot);
    return it;
  }

  /**
   * @brief  insert the given key into carmi
   *
   * @param data
   * @return true
   * @return false
   */
  bool Insert(DataType data) { return carmi_tree.Insert(data); }

  /**
   * @brief update the value of the given data
   *
   * @param data
   * @return true
   * @return false
   */
  bool Update(DataType data) { return carmi_tree.Update(data); }

  /**
   * @brief delete the record of the given key
   *
   * @param key
   * @return true
   * @return false
   */
  bool Delete(KeyType key) { return carmi_tree.Delete(key); }

  /**
   * @brief get the iterator of the first leaf node in carmi tree
   *
   * @return iterator
   */
  iterator begin() {
    iterator it;
    it.tree = this;
    it.currnode = carmi_tree.entireChild[carmi_tree.firstLeaf];
    it.currslot = 0;
    return it;
  }

  /**
   * @brief get the iterator of the last leaf node in carmi tree
   *
   * @return iterator
   */
  iterator end() {
    iterator it;
    it.tree = this;
    it.currnode = NULL;
    it.currslot = -1;
    return it;
  }

  /**
   * @brief calculate the space of carmi
   *
   * @return long double
   */
  long double CalculateSpace() const { return carmi_tree.CalculateSpace(); }

  /**
   * @brief print the structure of carmi
   *
   * @param level 1
   * @param type the root type
   * @param idx 0
   * @param levelVec the level of carmi
   * @param nodeVec the node type vector
   */
  void PrintStructure(int level, NodeType type, int idx,
                      std::vector<int> *levelVec,
                      std::vector<int> *nodeVec) const {
    carmi_tree.PrintStructure(level, type, idx, levelVec, nodeVec);
  }
};

template <typename KeyType, typename ValueType>
template <typename InputIterator>
void CARMICommon<KeyType, ValueType>::PreprocessInput(
    const InputIterator &first, const InputIterator &last,
    DataVectorType *initDataset) {
  InputIterator iter = first;
  while (iter != last) {
    initDataset->push_back(*iter);
    ++iter;
  }
}

template <typename KeyType, typename ValueType>
void CARMICommon<KeyType, ValueType>::Init(DataVectorType initDataset,
                                           double ratio, double insert_left,
                                           double insert_right, double rate) {
  DataVectorType findQuery = initDataset;
  DataVectorType insertQuery;
  std::vector<int> insertQueryIndex;

  // set the read frequency
  for (int i = 0; i < findQuery.size(); i++) {
    findQuery[i].second = 1;
  }

  // split insertQuery
  if (ratio != 1) {
    int cnt = round(1.0 / (1.0 - ratio));
    insert_left = initDataset.size() * insert_left + cnt - 1;
    insert_right = initDataset.size() * insert_right;
    for (int j = insert_left; j < insert_right; j += cnt) {
      insertQuery.push_back({initDataset[j].first, 1});
      insertQueryIndex.push_back(j);
    }
  }

  // construct carmi
  carmi_tree =
      carmi_impl(initDataset, findQuery, insertQuery, insertQueryIndex, rate);
  carmi_tree.Construction(initDataset, findQuery, insertQuery);
}

#endif  // SRC_INCLUDE_CARMI_COMMON_H_
