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
 * @brief The specific common version of the CARMI class template called
 * CARMICommon. This type of CARMI actually stores and manages data points in
 * the index structure.
 *
 * The implementation of common CARMI. This class provides users with basic
 * operations such as find, insert, update, and delete, as well as some basic
 * operations for the iterator of the CARMI index. Through the functions of
 * CARMICommon objects, users can automatically obtain indexes with good
 * performance for a given dataset without the need of manual tuning. In
 * addition, during initialization, users can set different lambda values to
 * control the different proportions of time and space costs, so as to achieve a
 * balance between the time and space cost under the corresponding parameters.
 *
 * The hybrid construction algorithm in CARMI can automatically construct the
 * optimal index structures under various datasets with the help of the given
 * historical find/insert queries. If there are no historical queries, users
 * only need to pass in the empty objects of the approriate type and CARMI will
 * default to a read-only workload to construct the optimal index structure
 * using the initDataset.
 *
 * @tparam KeyType the type of the key
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class CARMICommon {
 public:
  // *** Constructed Types

  /**
   * @brief the type of the implementation of CARMI
   */
  typedef CARMI<KeyType, ValueType> carmi_impl;

  /**
   * @brief the type of the data point: {key, value}
   */
  typedef typename CARMI<KeyType, ValueType>::DataType DataType;

  /**
   * @brief the type of the dataset: [{key_0, value_0}, {key_1, value_1}, ...,
   * {key_n, value_n}]. The dataset has been sorted.
   */
  typedef typename CARMI<KeyType, ValueType>::DataVectorType DataVectorType;

 private:
  /**
   * @brief The contained implementation object of CARMI
   */
  carmi_impl carmi_tree;

 public:
  //*** Constructor

  /**
   * @brief Construct a new CARMICommon object with the range
   * [initFirst, initLast) of initDataset and the range [insertFirst,
   * insertLast] of insertDataset.
   *
   * (1) The constructor uses the two input iterator of initFirst and initLast
   * to construct the init dataset to train the model in each node and construct
   * the optimal index structure on it. These data points will be actually
   * stored in the part of the index tree responsible for managing data points.
   * In addition, these data points will also be regarded as being accessed once
   * in historical find queries by default, providing a basis for the
   * calculation of the cost model.
   *
   * (2) Similarly, CARMI uses the two input iterator of insertFirst and
   * insertLast to construct the insert dataset. These data points will be
   * regarded as being inserted once in historical insert queries, and are used
   * to the calculation of the cost model.
   *
   * (3) The CARMI index structure can achieve a good performance under the
   * time/space setting based on the lambda parameter. The goal of the hybrid
   * construction algorithm is to minimize the total cost of historical access
   * and insert queries, which is: the average time cost of each query + lambda
   * times the space cost of the index tree.
   *
   * @tparam InputIterator the iterator type of input
   * @param[in] initFirst the first input iterator of initDataset, used to
   * construct the init dataset, which will be indexed. The initDataset is also
   * help for the calculation of the cost model.
   * @param[in] initLast the last input iterator of initoDataset, used to
   * construct the init dataset, which will be indexed.
   * @param[in] insertFirst the first input iterator of insertDataset, used to
   * construct the insert dataset. The insert dataset is used to calculate the
   * average insert time cost in the historical insert queries when calculating
   * the total cost model.
   * @param[in] insertLast the last input iterator of insertDataset, used to
   * construct the insert dataset.
   * @param[in] lambda lambda: cost = (time + lambda * space), used to tradeoff
   * between time and space cost in the cost model.
   */
  template <typename InputIterator>
  CARMICommon(const InputIterator &initFirst, const InputIterator &initLast,
              const InputIterator &insertFirst, const InputIterator &insertLast,
              double lambda) {
    // use the given two iterators to get the init dataset in the form of vector
    DataVectorType initDataset;
    PreprocessInput<InputIterator>(initFirst, initLast, &initDataset);
    // use the given two iterators to get the init dataset in the form of vector
    DataVectorType insertDataset;
    PreprocessInput<InputIterator>(insertFirst, insertLast, &insertDataset);
    // construct the findQuery/insertQuery using the two datasets and call the
    // construction function of carmi_tree to obtain the optimal index structure
    Init(initDataset, insertDataset, lambda);
  }

  /**
   * @brief the iterator of CARMICommon items. The iterator points to a specific
   * position of a data point in the carmi_tree and provides some basic
   * functions, such as: get the key value, get the data, get the next data
   * point and so on. Users can use this iterator to get the key/value of a data
   * point stored in the carmi_tree.
   */
  class iterator {
   public:
    /**
     * @brief Construct an empty new iterator object with the default values
     */
    inline iterator() : tree(NULL), currnode(NULL), currblock(0), currslot(0) {}

    /**
     * @brief Construct a new iterator object and set the pointer to the current
     * carmi index
     *
     * @param[in] t the pointer of the carmi tree
     */
    explicit inline iterator(CARMICommon *t)
        : tree(t), currnode(NULL), currblock(0), currslot(0) {}

    /**
     * @brief Construct a new iterator object
     *
     * @param[in] t the pointer of the carmi tree
     * @param[in] node the pointer of the current leaf node
     * @param[in] block the index of the current data block in the leaf node
     * @param[in] slot the index of the data points in the data block
     */
    inline iterator(CARMICommon *t, BaseNode<KeyType, ValueType> *node,
                    int block, int slot)
        : tree(t), currnode(node), currblock(block), currslot(slot) {}

    /**
     * @brief get the key value of this iterator
     *
     * @return const KeyType the key value
     * @retval DBL_MAX the data point is invalid
     */
    inline const KeyType key() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (currnode == NULL || tree == NULL || currblock < 0 || currslot < 0 ||
          currblock >= (currnode->cfArray.flagNumber & 0x00FFFFFF) ||
          currslot >= CFArrayType<KeyType, ValueType>::kMaxBlockCapacity) {
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
      if (currnode == NULL || tree == NULL || currblock < 0 || currslot < 0 ||
          currblock >= (currnode->cfArray.flagNumber & 0x00FFFFFF) ||
          currslot >= CFArrayType<KeyType, ValueType>::kMaxBlockCapacity) {
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
     * @param[in] x the given iterator
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
     * @param[in] x the given iterator
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
     */
    CARMICommon *tree;

    /**
     * @brief the pointer of the current leaf node
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
   * iterator and construct the dataset in the form of vector
   *
   * @tparam InputIterator the iterator type of input
   * @param[in] first the first input iterator
   * @param[in] last the last input iterator
   * @param[out] initDataset the constructed dataset: [{key_0, value_0}, {key_1,
   * value_1}, ..., {key_n, value_n}].
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
   * @brief construct the findQuery and insertQuery for the cost model and call
   * the hybrid construction algorithm to initialize the carmi_tree and
   * construct the optimal CARMI index structure
   *
   * @param[in] initDataset the init dataset: [{key_0, value_0}, {key_1,
   * value_1}, ..., {key_n, value_n}].
   * @param[in] insertDataset the insert dataset: [{key_0, value_0}, {key_1,
   * value_1}, ..., {key_n, value_n}].
   * @param[in] lambda lambda (time + lambda * space)
   */
  void Init(const DataVectorType &initDataset,
            const DataVectorType &insertDataset, double lambda) {
    DataVectorType findQuery = initDataset;
    DataVectorType insertQuery = insertDataset;
    std::vector<int> insertQueryIndex;

    // set the frequency, each data point is accessed once in the historical
    // find queries
    for (int i = 0; i < static_cast<int>(findQuery.size()); i++) {
      findQuery[i].second = 1;
    }
    // each data point is inserted once in the historical insert queries
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
   * @param[in] key the given key value
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
   * @param[in] datapoint the inserted data point
   * @retval true if the operation succeeds
   * @retval false if the operation fails
   */
  bool Insert(const DataType &datapoint) {
    return carmi_tree.Insert(datapoint);
  }

  /**
   * @brief update the value of the given data
   *
   * @param[in] datapoint the new data point
   * @retval true if the operation succeeds
   * @retval false if the operation fails
   */
  bool Update(const DataType &datapoint) {
    return carmi_tree.Update(datapoint);
  }

  /**
   * @brief delete the record of the given key
   *
   * @param[in] key the key value of the deleted record
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
    iterator it(this);
    it.currnode = carmi_tree.node.nodeArray[carmi_tree.firstLeaf];
    return it;
  }

  /**
   * @brief get the iterator of the last leaf node in carmi tree
   *
   * @return iterator
   */
  iterator end() { return iterator(this); }

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
