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
#ifndef SRC_INCLUDE_CARMI_COMMON_H_
#define SRC_INCLUDE_CARMI_COMMON_H_

#include <map>
#include <vector>

#include "./carmi.h"
#include "construct/construction.h"
#include "func/delete_function.h"
#include "func/find_function.h"
#include "func/insert_function.h"
#include "func/print_structure.h"
#include "func/update_function.h"
#include "memoryLayout/node_array.h"

/**
 * @brief the common type of CARMI
 *
 * @tparam KeyType the type of the key
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class CARMICommon {
 public:
  typedef CARMI<KeyType, ValueType>
      carmi_impl;  ///< the type of implementation of CARMI
  typedef typename CARMI<KeyType, ValueType>::DataType
      DataType;  ///< the type of the data point
  typedef typename CARMI<KeyType, ValueType>::DataVectorType
      DataVectorType;  ///< the type of the dataset

 private:
  carmi_impl carmi_tree;  ///< the implementation of CARMI

 public:
  /**
   * @brief Construct a new CARMICommon object with the range [first,last)
   *
   * @tparam InputIterator the iterator type of input
   * @param initFirst the first input iterator
   * @param initLast the last input iterator
   * @param insertFirst
   * @param insertLast
   * @param ratio readQuerySize/(readQuerySize+writeQuerySize)
   * @param lambda lambda (time + lambda * space)
   */
  template <typename InputIterator>
  CARMICommon(const InputIterator &initFirst, const InputIterator &initLast,
              const InputIterator &insertFirst, const InputIterator &insertLast,
              double ratio, double lambda) {
    DataVectorType initDataset;
    PreprocessInput<InputIterator>(initFirst, initLast, &initDataset);
    DataVectorType insertDataset;
    PreprocessInput<InputIterator>(insertFirst, insertLast, &insertDataset);
    Init(initDataset, insertDataset, ratio, lambda);
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
    inline iterator() : tree(NULL), currnode(NULL), currblock(0), currslot(0) {}

    /**
     * @brief Construct a new iterator object
     *
     * @param t the carmi tree
     */
    explicit inline iterator(CARMICommon *t)
        : tree(t), currnode(NULL), currblock(0), currslot(-1) {}

    /**
     * @brief Construct a new iterator object
     *
     * @param t the carmi tree
     * @param l the current node
     * @param s the current slot
     */
    inline iterator(CARMICommon *t, BaseNode<KeyType, ValueType> *l, int u,
                    int s)
        : tree(t), currnode(l), currblock(u), currslot(s) {}

    /**
     * @brief get the key of this iterator
     *
     * @return const KeyType
     */
    inline const KeyType key() const {
      if (currnode == NULL || tree == NULL ||
          currslot >= CFArrayType<KeyType, ValueType>::kMaxBlockCapacity ||
          currblock >= CFArrayType<KeyType, ValueType>::kMaxBlockNum) {
        return DBL_MAX;
      }
      int left = currnode->cfArray.m_left;
      if (left + currblock >= tree->carmi_tree.data.usedDatasize) {
        std::cout << "index is: " << left + currblock << std::endl;
        std::cout << "the index is out of range, size:"
                  << tree->carmi_tree.data.usedDatasize << std::endl;
      }
      return tree->carmi_tree.data.dataArray[left + currblock]
          .slots[currslot]
          .first;
    }

    /**
     * @brief get the data of this iterator
     *
     * @return const ValueType
     */
    inline const ValueType data() const {
      if (currnode == NULL || tree == NULL ||
          currslot >= CFArrayType<KeyType, ValueType>::kMaxBlockCapacity ||
          currblock >= CFArrayType<KeyType, ValueType>::kMaxBlockNum) {
        return DBL_MAX;
      }
      int left = currnode->cfArray.m_left;
      if (left + currblock >= tree->carmi_tree.data.usedDatasize) {
        std::cout << "index is: " << left + currblock << std::endl;
        std::cout << "the index is out of range, size:"
                  << tree->carmi_tree.data.usedDatasize << std::endl;
      }
      return tree->carmi_tree.data.dataArray[left + currblock]
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
             (x.currslot == currslot && x.currblock == currblock);
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
             (x.currslot != currslot || x.currblock != currblock);
    }

    /**
     * @brief get the iterator of the next slot
     *
     * @return iterator&
     */
    inline iterator &operator++() {
      int left = currnode->cfArray.m_left;
      currslot++;
      while (currblock < (currnode->cfArray.flagNumber & 0x00FFFFFF) ||
             currnode->cfArray.nextLeaf != -1) {
        if (left + currblock >= tree->carmi_tree.data.dataArray.size()) {
          *this = this->tree->end();
          return *this;
        } else if (currblock == (currnode->cfArray.flagNumber & 0x00FFFFFF)) {
          currnode =
              &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
          currslot = 0;
          currblock = 0;
          left = currnode->cfArray.m_left;
          continue;
        } else if (
            // currslot == tree->carmi_tree.kMaxDataPointNum ||
            tree->carmi_tree.data.dataArray[left + currblock]
                .slots[currslot]
                .first == DBL_MAX) {
          currslot = 0;
          currblock++;
        } else if (tree->carmi_tree.data.dataArray[left + currblock]
                       .slots[currslot]
                       .first != DBL_MAX) {
          return *this;
        }
      }
      *this = this->tree->end();
      return *this;
    }

    CARMICommon *tree;                       ///< the carmi tree
    BaseNode<KeyType, ValueType> *currnode;  ///< the current leaf node
    int currblock;  ///< index in unions of this leaf node
    int currslot;   ///< index in external_data
  };

 private:
  /**
   * @brief preprocess the input between the first iterator and the last
   * iterator
   *
   * @tparam InputIterator the iterator type of input
   * @param first the first input iterator
   * @param last the last input iterator
   * @param initDataset the dataset
   */
  template <typename InputIterator>
  void PreprocessInput(const InputIterator &first, const InputIterator &last,
                       DataVectorType *initDataset);

  /**
   * @brief split the dataset into read queries and write queris according to
   * the ratio, the range of write queries is [insert_leaf*dataset.size(),
   * insert_right*dataset.size())
   *
   * @param initDataset the dataset
   * @param insertDataset
   * @param ratio readQuerySize/(readQuerySize+writeQuerySize)
   * @param lambda lambda (time + lambda * space)
   */
  void Init(const DataVectorType &initDataset,
            const DataVectorType &insertDataset, double ratio, double lambda);

  // main functions
 public:
  /**
   * @brief find the corresponding iterator of the given key
   *
   * @param key the given key value
   * @return iterator
   */
  iterator Find(const KeyType &key) {
    iterator it(this);
    it.currnode = carmi_tree.Find(key, &it.currblock, &it.currslot);
    return it;
  }

  /**
   * @brief insert the given key into carmi
   *
   * @param datapoint the inserted data point
   * @retval true if the operation succeeds
   * @retval false if the operation fails
   */
  bool Insert(const DataType &datapoint) {
    return carmi_tree.Insert(datapoint);
  }

  /**
   * @brief update the value of the given data
   *
   * @param datapoint the new data point
   * @retval true if the operation succeeds
   * @retval false if the operation fails
   */
  bool Update(const DataType &datapoint) {
    return carmi_tree.Update(datapoint);
  }

  /**
   * @brief delete the record of the given key
   *
   * @param key the key value of the deleted record
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
   * @brief print the structure of carmi
   *
   * @param level 1
   * @param type the root type
   * @param dataSize the size of the dataset
   * @param idx 0
   * @param levelVec the level of carmi
   * @param nodeVec the node type vector
   */
  void PrintStructure(int level, NodeType type, int dataSize, int idx,
                      std::vector<int> *levelVec, std::vector<int> *nodeVec) {
    carmi_tree.PrintStructure(level, type, dataSize, idx, levelVec, nodeVec);
    std::cout << "avg level is: " << carmi_tree.sumDepth << std::endl;
    // std::cout << "avg level is: "
    //           << carmi_tree.sumDepth * 1.0 / carmi_tree.nowChildNumber
    //           << std::endl;
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
void CARMICommon<KeyType, ValueType>::Init(const DataVectorType &initDataset,
                                           const DataVectorType &insertDataset,
                                           double ratio, double rate) {
  DataVectorType findQuery = initDataset;
  DataVectorType insertQuery = insertDataset;
  std::vector<int> insertQueryIndex;

  // set the read frequency
  for (int i = 0; i < static_cast<int>(findQuery.size()); i++) {
    findQuery[i].second = 1;
  }
  for (int i = 0; i < static_cast<int>(insertDataset.size()); i++) {
    insertQuery[i].second = 1;
  }

  // construct carmi
  carmi_tree = carmi_impl(initDataset, findQuery, insertQuery, rate);
  carmi_tree.Construction();
}

#endif  // SRC_INCLUDE_CARMI_COMMON_H_
