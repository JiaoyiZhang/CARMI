/**
 * @file carmi_common.h
 * @author Jiaoyi
 * @brief the common type of CARMI
 * @version 3.0
 * @date 2021-04-01
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CARMI_COMMON_H_
#define CARMI_COMMON_H_

#include <map>
#include <vector>

#include "./carmi.h"
#include "construct/construction.h"
#include "func/delete_function.h"
#include "func/find_function.h"
#include "func/get_node_info.h"
#include "func/insert_function.h"
#include "func/update_function.h"
#include "memoryLayout/node_array.h"

/**
 * @brief The specific common version of the CARMI class template.
 *
 * The implementation of common CARMI. This class provides users with basic
 * operations such as find, insert, update, and delete, as well as some basic
 * operations for the iterator of the CARMI index. Through the functions of
 * CARMICommon objects, users can construct indexes with good performance for a
 * given dataset without the need of too much space. In addition, during
 * initialization, users can set different lambda values to control the
 * different proportions of time and space costs, so as to achieve a balance
 * between the time and space cost under the corresponding parameters.
 *
 * @tparam KeyType the type of the key
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class CARMICommon {
 public:
  // *** Constructed Types

  /**
   * @brief the type of implementation of CARMI
   *
   */
  typedef CARMI<KeyType, ValueType> carmi_impl;

  /**
   * @brief the type of the data point
   *
   */
  typedef typename CARMI<KeyType, ValueType>::DataType DataType;

  /**
   * @brief the type of the dataset
   *
   */
  typedef typename CARMI<KeyType, ValueType>::DataVectorType DataVectorType;

 private:
  /**
   * @brief The contained implementation object of CARMI
   *
   */
  carmi_impl carmi_tree;

 public:
  //*** Constructor

  /**
   * @brief Construct a new CARMICommon object with the range
   * [initFirst, initLast) of initDataset and the range [insertFirst,
   * insertLast] of insertDataset.
   *
   * @tparam InputIterator the iterator type of input
   * @param initFirst[in] the first input iterator of initDataset
   * @param initLast[in] the last input iterator of initDataset
   * @param insertFirst[in] the first input iterator of insertDataset
   * @param insertLast[in] the last input iterator of insertDataset
   * @param lambda[in] lambda (time + lambda * space), used to tradeoff between
   * time and space cost
   */
  template <typename InputIterator>
  CARMICommon(const InputIterator &initFirst, const InputIterator &initLast,
              const InputIterator &insertFirst, const InputIterator &insertLast,
              double lambda) {
    DataVectorType initDataset;
    PreprocessInput<InputIterator>(initFirst, initLast, &initDataset);
    DataVectorType insertDataset;
    PreprocessInput<InputIterator>(insertFirst, insertLast, &insertDataset);
    Init(initDataset, insertDataset, lambda);
  }

  /**
   * @brief the iterator of CARMICommon items. The iterator points to a specific
   * position of a data point in the carmi_tree.
   */
  class iterator {
   public:
    /**
     * @brief Construct an empty new iterator object
     */
    inline iterator() : tree(NULL), currnode(NULL), currblock(0), currslot(0) {}

    /**
     * @brief Construct a new iterator object
     *
     * @param t[in] the pointer of the carmi tree
     */
    explicit inline iterator(CARMICommon *t)
        : tree(t), currnode(NULL), currblock(0), currslot(-1) {}

    /**
     * @brief Construct a new iterator object
     *
     * @param t[in] the pointer of the carmi tree
     * @param n[in] the pointer of the current leaf node
     * @param u[in] the index of the current data block in the leaf node
     * @param s[in] the index of the data points in the data block
     */
    inline iterator(CARMICommon *t, BaseNode<KeyType, ValueType> *n, int b,
                    int s)
        : tree(t), currnode(n), currblock(b), currslot(s) {}

    /**
     * @brief get the key value of this iterator
     *
     * @return const KeyType the key value
     * @retval DBL_MAX the data point is invalid
     */
    inline const KeyType key() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (currnode == NULL || tree == NULL ||
          currslot >= CFArrayType<KeyType, ValueType>::kMaxBlockCapacity ||
          currblock >= CFArrayType<KeyType, ValueType>::kMaxBlockNum) {
        return DBL_MAX;
      }
      // Case 2: the current iterator is valid, get the index of the data block
      // and return the key value in the data block
      int left = currnode->cfArray.m_left;
      return tree->carmi_tree.data.dataArray[left + currblock]
          .slots[currslot]
          .first;
    }

    /**
     * @brief get the data value of this iterator
     *
     * @return const ValueType the data value
     * @retval DBL_MAX the data point is invalid
     */
    inline const ValueType data() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (currnode == NULL || tree == NULL ||
          currslot >= CFArrayType<KeyType, ValueType>::kMaxBlockCapacity ||
          currblock >= CFArrayType<KeyType, ValueType>::kMaxBlockNum) {
        return DBL_MAX;
      }
      // Case 2: the current iterator is valid, get the index of the data block
      // and return the data value in the data block
      int left = currnode->cfArray.m_left;
      return tree->carmi_tree.data.dataArray[left + currblock]
          .slots[currslot]
          .second;
    }

    /**
     * @brief check if the given iterator x is equal to this iterator
     *
     * @param x[in] the given iterator
     * @return true the given iterator is equal to this iterator
     * @return false the given iterator is unequal to this iterator
     */
    inline bool operator==(const iterator &x) const {
      return (x.tree == tree && x.currnode == currnode) &&
             (x.currslot == currslot && x.currblock == currblock);
    }

    /**
     * @brief check if the given iterator x is unequal to this iterator
     *
     * @param x[in] the given iterator
     * @return true the given iterator is unequal to this iterator
     * @return false the given iterator is equal to this iterator
     */
    inline bool operator!=(const iterator &x) const {
      return (x.currnode != currnode || x.tree != tree) ||
             (x.currslot != currslot || x.currblock != currblock);
    }

    /**
     * @brief get the iterator of the next data point
     *
     * @return iterator& the next iterator
     */
    inline iterator &operator++() {
      int left = currnode->cfArray.m_left;
      currslot++;
      while (currblock < (currnode->cfArray.flagNumber & 0x00FFFFFF) ||
             currnode->cfArray.nextLeaf != -1) {
        // Case 1: the next data block is invalid, return end()
        if (left + currblock >= tree->carmi_tree.data.dataArray.size()) {
          *this = this->tree->end();
          return *this;
        } else if (currblock == (currnode->cfArray.flagNumber & 0x00FFFFFF)) {
          // Case 2: the next data point is stored in the next leaf node,
          // update the variables and continue to find the next data point
          currnode =
              &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
          currslot = 0;
          currblock = 0;
          left = currnode->cfArray.m_left;
          continue;
        } else if (tree->carmi_tree.data.dataArray[left + currblock]
                       .slots[currslot]
                       .first == DBL_MAX) {
          // Case 3: the next data point is stored in the next data block
          currslot = 0;
          currblock++;
        } else if (tree->carmi_tree.data.dataArray[left + currblock]
                       .slots[currslot]
                       .first != DBL_MAX) {
          // Case 4: the next data point is stored in the same data block,
          // return it
          return *this;
        }
      }
      // Case 5: the next data point is invalid, return end()
      *this = this->tree->end();
      return *this;
    }

   public:
    //*** Public Data Members of Iterator Objects

    /**
     * @brief the pointer of the carmi tree
     *
     */
    CARMICommon *tree;

    /**
     * @brief the pointer of the current leaf node
     *
     */
    BaseNode<KeyType, ValueType> *currnode;

    /**
     * @brief the index of the current data block in the leaf node
     */
    int currblock;

    /**
     * @brief the index of the data point in the data block
     */
    int currslot;
  };

 private:
  //*** The Private Functions of Constructor

  /**
   * @brief preprocess the input dataset between the first iterator and the last
   * iterator
   *
   * @tparam InputIterator the iterator type of input
   * @param first[in] the first input iterator
   * @param last[in] the last input iterator
   * @param initDataset[out] the dataset
   */
  template <typename InputIterator>
  void PreprocessInput(const InputIterator &first, const InputIterator &last,
                       DataVectorType *initDataset) {
    InputIterator iter = first;
    while (iter != last) {
      initDataset->push_back(*iter);
      ++iter;
    }
  }

  /**
   * @brief initialize the carmi_tree
   *
   * @param initDataset[in] the init dataset
   * @param insertDataset[in] the insert dataset
   * @param lambda[in] lambda (time + lambda * space)
   */
  void Init(const DataVectorType &initDataset,
            const DataVectorType &insertDataset, double lambda) {
    DataVectorType findQuery = initDataset;
    DataVectorType insertQuery = insertDataset;
    std::vector<int> insertQueryIndex;

    // set the frequency
    for (int i = 0; i < static_cast<int>(findQuery.size()); i++) {
      findQuery[i].second = 1;
    }
    for (int i = 0; i < static_cast<int>(insertDataset.size()); i++) {
      insertQuery[i].second = 1;
    }

    // construct carmi
    carmi_tree = carmi_impl(initDataset, findQuery, insertQuery, lambda);
    carmi_tree.Construction();
  }

 public:
  // *** Basic Functions of CARMI Common Objects

  /**
   * @brief find the corresponding iterator of the given key value
   *
   * @param key[in] the given key value
   * @return iterator the iterator of the data point
   */
  iterator Find(const KeyType &key) {
    iterator it(this);
    it.currnode = carmi_tree.Find(key, &it.currblock, &it.currslot);
    return it;
  }

  /**
   * @brief insert the given data point into carmi
   *
   * @param datapoint[in] the inserted data point
   * @retval true if the operation succeeds
   * @retval false if the operation fails
   */
  bool Insert(const DataType &datapoint) {
    return carmi_tree.Insert(datapoint);
  }

  /**
   * @brief update the value of the given data
   *
   * @param datapoint[in] the new data point
   * @retval true if the operation succeeds
   * @retval false if the operation fails
   */
  bool Update(const DataType &datapoint) {
    return carmi_tree.Update(datapoint);
  }

  /**
   * @brief delete the record of the given key
   *
   * @param key[in] the key value of the deleted record
   * @retval true if the operation succeeds
   * @retval false if the operation fails
   */
  bool Delete(const KeyType &key) { return carmi_tree.Delete(key); }

  /**
   * @brief get the iterator of the first leaf node in carmi tree
   *
   * @return iterator
   */
  iterator begin() {
    iterator it;
    it.tree = this;
    it.currnode = carmi_tree.node.nodeArray[carmi_tree.firstLeaf];
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
   * @return long double: space
   */
  long double CalculateSpace() const { return carmi_tree.CalculateSpace(); }

  /**
   * @brief Get the information of the tree node, return the type identifier of
   * this node, the number of its child nodes and the starting index of the
   * first child node in the node array.
   *
   * @param[in] idx the index of the node in the node array
   * @param[out] childNumber the number of the child nodes of this node
   * @param[out] childStartIndex the starting index of the first child node
   * @return int the type identifier of this node
   */
  int GetNodeInfo(int idx, int *childNumber, int *childStartIndex) {
    return carmi_tree.GetNodeInfo(idx, childNumber, childStartIndex);
  }
};

#endif  // CARMI_COMMON_H_
