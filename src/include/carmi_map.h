/**
 * @file carmi_map.h
 * @author Jiaoyi
 * @brief Implements the STL map using a CARMI tree.
 * @version 3.0
 * @date 2021-04-01
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CARMI_MAP_H_
#define CARMI_MAP_H_

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "./carmi.h"
#include "construct/construction.h"
#include "func/delete_function.h"
#include "func/find_function.h"
#include "func/get_node_info.h"
#include "func/insert_function.h"
#include "memoryLayout/node_array.h"

/**
 * @brief The specific common version of the CARMI class template called
 * CARMIMap. This type of CARMI actually stores and manages data points in
 * the index structure.
 *
 * Implements the STL map using a CARMI tree. It can be used as a drop-in
 * replacement for std::map.
 *
 * This class provides users with basic operations such as find, insert, and
 * some basic operations for the iterator of the CARMI index. Through the
 * functions of CARMIMap objects, users can automatically obtain indexes with
 * good performance for a given dataset without manual tuning. In addition,
 * during initialization, users can set different lambda values to control the
 * different proportions of time and space costs to achieve a balance between
 * the time and space cost under the corresponding parameters.
 *
 * The hybrid construction algorithm in CARMI can automatically construct the
 * optimal index structures under various datasets with the help of the given
 * historical find/insert queries. If there are no historical queries, CARMI
 * will default to a read-only workload to construct the optimal index structure
 * using the initDataset.
 *
 * @tparam KeyType Type of keys.
 * @tparam ValueType Type of values of data points.
 * @tparam Compare A binary predicate that takes two element keys as arguments
 * and returns a bool.
 * @tparam Alloc Type of the allocator object used to define the storage
 * allocation model.
 */
template <typename KeyType, typename ValueType,
          typename Compare = std::less<KeyType>,
          typename Alloc = std::allocator<LeafSlots<KeyType, ValueType>>>
class CARMIMap {
 public:
  // *** Constructed Types

  /**
   * @brief The type of the implementation of CARMI.
   */
  typedef CARMI<KeyType, ValueType, Compare, Alloc> carmi_impl;

  /**
   * @brief The type of the data point: {key, value}.
   */
  typedef typename CARMI<KeyType, ValueType, Compare, Alloc>::DataType DataType;

  /**
   * @brief The type of the dataset: [{key_0, value_0}, {key_1, value_1}, ...,
   * {key_n, value_n}]. The dataset has been sorted.
   */
  typedef typename CARMI<KeyType, ValueType, Compare, Alloc>::DataVectorType
      DataVectorType;

  /**
   * @brief The type of the key value vector: [key_0, key_1, ..., key_n].
   */
  typedef typename CARMI<KeyType, ValueType, Compare, Alloc>::KeyVectorType
      KeyVectorType;

  /**
   * @brief The type of the historical queries vector: [{key_0, times_0},
   * {key_1, times_1}, ..., {key_n, times_n}]. The second object means the
   * accessed times of the key value in the historical access queries.
   */
  typedef
      typename CARMI<KeyType, ValueType, Compare, Alloc>::QueryType QueryType;

 private:
  /**
   * @brief The contained implementation object of CARMI
   */
  carmi_impl carmi_tree;

 public:
  // *** Constructors and Destructor

  /**
   * @brief Construct a new empty CARMIMap object.
   */
  CARMIMap() : carmi_tree() {}

  /**
   * @brief Destroy the CARMIMap object.
   */
  ~CARMIMap() {}

  /**
   * @brief Construct a new empty CARMIMap object with the standard key
   * comparison function.
   *
   * @param[in] alloc The allocator object used to define the storage allocation
   * model.
   */
  explicit CARMIMap(const Alloc &alloc) { carmi_tree.allocator_ = alloc; }

  /**
   * @brief Construct a new empty CARMIMap object with a special key comparision
   * object.
   *
   * @param[in] comp A binary predicate that takes two element keys as arguments
   * and returns a bool.
   * @param[in] alloc The allocator object used to define the storage allocation
   * model.
   */
  explicit CARMIMap(const Compare &comp, const Alloc &alloc) {
    carmi_tree.key_less_ = comp;
    carmi_tree.allocator_ = alloc;
  }

  /**
   * @brief Construct a new CARMIMap object with the range [first, last)
   * of initDataset. The range does not need to be sorted. This constructor has
   * no historical insert query by default, indicating that it is currently a
   * read-only workload.
   *
   * (1) The constructor uses the two input iterators of first and last to
   * construct the init dataset to train the model in each node and construct
   * the optimal index structure on it. These data points will be actually
   * stored in the part of the index tree responsible for managing data points.
   * In addition, these data points will also be regarded as being accessed once
   * in historical find queries by default, providing a basis for the
   * calculation of the cost model.
   *
   * (2) The CARMI index structure can perform well under the time/space setting
   * based on the lambda parameter. The goal of the hybrid construction
   * algorithm is to minimize the total cost of historical access queries: the
   * average time cost of each query + lambda times the space cost of the index
   * tree.
   *
   * @tparam InputIterator
   * @param[in] first the first input iterator of initDataset, used to
   * construct the init dataset, which will be indexed. The initDataset is also
   * help for the calculation of the cost model.
   * @param[in] last the last input iterator of initDataset, used to
   * construct the init dataset, which will be indexed.
   * @param[in] lambda lambda: cost = (time + lambda * space), used to tradeoff
   * between time and space cost in the cost model.
   * @param[in] comp A binary predicate that takes two element keys as arguments
   * and returns a bool.
   * @param[in] alloc The allocator object used to define the storage allocation
   * model.
   */
  template <class InputIterator>
  CARMIMap(const InputIterator &first, const InputIterator &last, double lambda,
           const Compare &comp = Compare(), const Alloc &alloc = Alloc()) {
    carmi_tree.key_less_ = comp;
    carmi_tree.allocator_ = alloc;
    // use the given two iterators to get the init dataset in the form of vector
    DataVectorType initDataset;
    PreprocessInput<InputIterator>(first, last, &initDataset);
    DataVectorType insertDataset;
    Init(initDataset, insertDataset, lambda);
  }

  /**
   * @brief Construct a new CARMIMap object with the range [initFirst, initLast)
   * of initDataset and the range [insertFirst, insertLast] of insertDataset.
   * The range does not need to be sorted.
   *
   * (1) The constructor uses the two input iterator of initFirst and initLast
   * to construct the init dataset to train the model in each node and construct
   * the optimal index structure. These data points will be actually stored in
   * the part of the index tree responsible for managing data points. In
   * addition, these data points will also be regarded as being accessed once in
   * historical find queries by default, providing a basis for the calculation
   * of the cost model.
   *
   * (2) Similarly, CARMI uses the two input iterator of insertFirst and
   * insertLast to construct the insert dataset. These data points will be
   * regarded as being inserted once in historical insert queries and used to
   * calculate the cost model.
   *
   * (3) The CARMI index structure can perform well under the time/space setting
   * based on the lambda parameter. The goal of the hybrid construction
   * algorithm is to minimize the total cost of historical access and insert
   * queries, which is: the average time cost of each query + lambda times the
   * space cost of the index tree.
   *
   * @tparam InputIterator the iterator type of input
   * @param[in] initFirst the first input iterator of initDataset, used to
   * construct the init dataset, which will be indexed. The initDataset is also
   * help for the calculation of the cost model.
   * @param[in] initLast the last input iterator of initDataset, used to
   * construct the init dataset, which will be indexed.
   * @param[in] insertFirst the first input iterator of insertDataset, used to
   * construct the insert dataset. The insert dataset is used to calculate the
   * average insert time cost in the historical insert queries when calculating
   * the total cost model.
   * @param[in] insertLast the last input iterator of insertDataset, used to
   * construct the insert dataset.
   * @param[in] lambda lambda: cost = (time + lambda * space), used to tradeoff
   * between time and space cost in the cost model.
   * @param[in] comp A binary predicate that takes two element keys as arguments
   * and returns a bool.
   * @param[in] alloc The allocator object used to define the storage allocation
   * model.
   */
  template <typename InputIterator>
  CARMIMap(const InputIterator &initFirst, const InputIterator &initLast,
           const InputIterator &insertFirst, const InputIterator &insertLast,
           double lambda, const Compare &comp = Compare(),
           const Alloc &alloc = Alloc()) {
    carmi_tree.key_less_ = comp;
    carmi_tree.allocator_ = alloc;
    // use the given two iterators to get the init dataset in the form of vector
    DataVectorType initDataset;
    PreprocessInput<InputIterator>(initFirst, initLast, &initDataset);
    // use the given two iterators to get the insert dataset in the form of
    // vector
    DataVectorType insertDataset;
    PreprocessInput<InputIterator>(insertFirst, insertLast, &insertDataset);
    // construct the findQuery/insertQuery using the two datasets and call the
    // construction function of carmi_tree to obtain the optimal index structure
    Init(initDataset, insertDataset, lambda);
  }

 public:
  /// *** Fast Copy: Assign Operator and Copy Constructors

  /**
   * @brief Construct a new CARMIMap object using the copy constructor.
   *
   * @param[in] x the other carmi map object
   */
  CARMIMap(const CARMIMap &x) : carmi_tree(x.carmi_tree) {}

  /**
   * @brief Construct a new CARMIMap object using the copy constructor.
   *
   * @param[in] x the other carmi map object
   * @param[in] alloc The allocator object used to define the storage allocation
   * model.
   */
  CARMIMap(const CARMIMap &x, const Alloc &alloc) : carmi_tree(x.carmi_tree) {
    carmi_tree.allocator_ = alloc;
  }

  /**
   * @brief Assignment operator. All the key/data pairs are copied.
   *
   * @param[in] other the given carmi map object
   * @return CARMIMap& this object
   */
  inline CARMIMap &operator=(const CARMIMap &other) {
    if (this != &other) {
      carmi_tree = other.carmi_tree;
    }
    return *this;
  }

  class iterator;
  class const_iterator;
  class reverse_iterator;
  class const_reverse_iterator;

  /**
   * @brief The iterator of CARMIMap items. The iterator points to a specific
   * data point in the carmi_tree and provides some basic functions as the
   * STL::map.
   */
  class iterator {
   public:
    /**
     * @brief Construct an empty new iterator object with the default values.
     */
    inline iterator() : tree(NULL), currnode(NULL), currblock(0), currslot(0) {}

    /**
     * @brief Construct a new iterator object and set the pointer to the current
     * carmi index.
     *
     * @param[in] t the pointer of the carmi tree
     */
    explicit inline iterator(CARMIMap *t)
        : tree(t), currnode(NULL), currblock(0), currslot(0) {}

    /**
     * @brief Construct a new iterator object.
     *
     * @param[in] t the pointer of the carmi tree
     * @param[in] node the pointer of the current leaf node
     * @param[in] block the index of the current data block in the leaf node
     * @param[in] slot the index of the data points in the data block
     */
    inline iterator(CARMIMap *t,
                    BaseNode<KeyType, ValueType, Compare, Alloc> *node,
                    int block, int slot)
        : tree(t), currnode(node), currblock(block), currslot(slot) {}

    /**
     * @brief Construct a new iterator object from a reverse iterator.
     *
     * @param[in] it the given reverse iterator
     */
    explicit inline iterator(const reverse_iterator &it)
        : tree(it.tree),
          currnode(it.currnode),
          currblock(it.currblock),
          currslot(it.currslot) {}

    /**
     * @brief Get the key value of this iterator.
     *
     * @return const KeyType& the key value
     */
    inline const KeyType &key() const {
      // return the key value in the data block
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot]
          .first;
    }

    /**
     * @brief Get the data value of this iterator.
     *
     * @return const ValueType& the data value
     */
    inline const ValueType &data() const {
      // return the data value in the data block
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot]
          .second;
    }

    /**
     * @brief Return a reference of the data point of this iterator.
     *
     * @return DataType& the reference of this data point
     */
    inline DataType &operator*() const {
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot];
    }

    /**
     * @brief Return the pointer to this data point.
     *
     * @return DataType* the pointer to this data point
     */
    inline DataType *operator->() const {
      return &(
          tree->carmi_tree.data.dataArray[currnode->cfArray.m_left + currblock]
              .slots[currslot]);
    }

    /**
     * @brief Check if the given iterator x is equal to this iterator.
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is equal to this iterator
     * @retval false the given iterator is unequal to this iterator
     */
    inline bool operator==(const iterator &x) const {
      return (x.tree == tree && x.currnode == currnode) &&
             (x.currslot == currslot && x.currblock == currblock);
    }

    /**
     * @brief Check if the given iterator x is unequal to this iterator.
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is unequal to this iterator
     * @retval false the given iterator is equal to this iterator
     */
    inline bool operator!=(const iterator &x) const {
      return (x.currnode != currnode || x.tree != tree) ||
             (x.currslot != currslot || x.currblock != currblock);
    }

    /**
     * @brief Prefix++, get the iterator of the next data point.
     *
     * @return iterator& the next iterator
     */
    inline iterator &operator++() {
      // Case 1: the next data point is stored in the same data block,
      // return it
      bool isSuccess = advanceSlot();
      if (isSuccess) {
        return *this;
      }

      // Case 2: the next data point is stored in the next data block
      isSuccess = advanceBlock();
      if (isSuccess) {
        return *this;
      }

      // Case 3: the next data point is stored in the next leaf node,
      // update the variables and continue to find the next data point
      isSuccess = advanceNode();
      if (isSuccess) {
        return *this;
      }
      // Case 4: the next data point is invalid, return end()
      *this = this->tree->end();
      return *this;
    }

    /**
     * @brief Postfix++, get the iterator of the next data point.
     *
     * @return iterator& the next iterator
     */
    inline iterator operator++(int) {
      iterator tmp = *this;  // copy ourselves
      // Case 1: the next data point is stored in the same data block,
      // return it
      bool isSuccess = advanceSlot();
      if (isSuccess) {
        return tmp;
      }

      // Case 2: the next data point is stored in the next data block
      isSuccess = advanceBlock();
      if (isSuccess) {
        return tmp;
      }

      // Case 3: the next data point is stored in the next leaf node,
      // update the variables and continue to find the next data point
      isSuccess = advanceNode();
      if (isSuccess) {
        return tmp;
      }
      // Case 4: the next data point is invalid, return end()
      *this = this->tree->end();
      return tmp;
    }

    /**
     * @brief Prefix--, get the iterator of the previous data point.
     *
     * @return iterator& the previous iterator
     */
    inline iterator &operator--() {
      // Case 1: the previous data point is stored in the same data block,
      // return it
      bool isSuccess = backwardSlot();
      if (isSuccess) {
        return *this;
      }

      // Case 2: the previous data point is stored in the previous data block
      isSuccess = backwardBlock();
      if (isSuccess) {
        return *this;
      }

      // Case 3: the previous data point is stored in the previous leaf node,
      // update the variables and continue to find the previous data point
      isSuccess = backwardNode();
      if (isSuccess) {
        return *this;
      }
      // Case 4: the previous data point is invalid, return end()
      *this = this->tree->end();
      return *this;
    }

    /**
     * @brief Postfix--, get the iterator of the previous data point.
     *
     * @return iterator& the previous iterator
     */
    inline iterator operator--(int) {
      iterator tmp = *this;  // copy ourselves
      // Case 1: the previous data point is stored in the same data block,
      // return it
      bool isSuccess = backwardSlot();
      if (isSuccess) {
        return tmp;
      }

      // Case 2: the previous data point is stored in the previous data block
      isSuccess = backwardBlock();
      if (isSuccess) {
        return tmp;
      }

      // Case 3: the previous data point is stored in the previous leaf node,
      // update the variables and continue to find the previous data point
      isSuccess = backwardNode();
      if (isSuccess) {
        return tmp;
      }
      // Case 4: the previous data point is invalid, return end()
      *this = this->tree->end();
      return tmp;
    }

   private:
    //*** Private Functions of Iterator Objects

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the same data
     * block.
     *
     * @retval true point to the next data point
     * @retval false the next data point is not stored in the same data block
     */
    inline bool advanceSlot() {
      currslot++;
      if (currslot < currnode->cfArray.GetBlockSize(currblock)) {
        return true;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the next data
     * block.
     *
     * @retval true point to the next data point
     * @retval false the next data point is not stored in the netx data block
     */
    inline bool advanceBlock() {
      currblock++;
      while (currblock < (currnode->cfArray.flagNumber & 0x00FFFFFF)) {
        currslot = -1;
        bool isSuccess = advanceSlot();
        if (isSuccess) {
          return true;
        }
        currblock++;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the next leaf node.
     *
     * @retval true point to the next data point
     * @retval false the current data point is the last one
     */
    inline bool advanceNode() {
      while (currnode->cfArray.nextLeaf != -1) {
        currnode =
            &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
        while ((currnode->cfArray.flagNumber & 0x00FFFFFF) == 0) {
          if (currnode->cfArray.nextLeaf == -1) {
            return false;
          }
          currnode =
              &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
        }
        currblock = -1;
        bool isSuccess = advanceBlock();
        if (isSuccess) {
          return true;
        }
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the same
     * data block.
     *
     * @retval true point to the previous data point
     * @retval false the previous data point is not stored in the same data
     * block
     */
    inline bool backwardSlot() {
      currslot--;
      if (currslot >= 0) {
        return true;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the
     * previous data block.
     *
     * @retval true point to the previous data point
     * @retval false the previous data point is not stored in the netx data
     * block
     */
    inline bool backwardBlock() {
      currblock--;
      while (currblock >= 0) {
        currslot = currnode->cfArray.GetBlockSize(currblock);
        bool isSuccess = backwardSlot();
        if (isSuccess) {
          return true;
        }
        currblock--;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the
     * previous leaf node.
     *
     * @retval true point to the previous data point
     * @retval false the current data point is the last one
     */
    inline bool backwardNode() {
      while (currnode->cfArray.previousLeaf != -1) {
        currnode =
            &(tree->carmi_tree.node.nodeArray[currnode->cfArray.previousLeaf]);
        while ((currnode->cfArray.flagNumber & 0x00FFFFFF) == 0) {
          if (currnode->cfArray.previousLeaf == -1) {
            return false;
          }
          currnode = &(
              tree->carmi_tree.node.nodeArray[currnode->cfArray.previousLeaf]);
        }
        currblock = (currnode->cfArray.flagNumber & 0x00FFFFFF);
        bool isSuccess = backwardBlock();
        if (isSuccess) {
          return true;
        }
      }
      return false;
    }

   private:
    //*** Private Data Members of Iterator Objects

    /**
     * @brief The pointer of the carmi tree.
     */
    CARMIMap *tree;

    /**
     * @brief The pointer of the current leaf node.
     */
    BaseNode<KeyType, ValueType, Compare, Alloc> *currnode;

    /**
     * @brief The index of the current data block in the leaf node.
     */
    int currblock;

    /**
     * @brief The index of the data point in the data block.
     */
    int currslot;

    friend class const_iterator;
    friend class reverse_iterator;
    friend class const_reverse_iterator;
    friend class CARMIMap;
  };

  /**
   * @brief The const iterator of CARMIMap items. The iterator points to a
   * specific data point in the carmi_tree and provides some basic functions as
   * the STL::map.
   */
  class const_iterator {
   public:
    /**
     * @brief Construct an empty new const iterator object with the default
     * values.
     */
    inline const_iterator() {}

    /**
     * @brief Construct a new const iterator object and set the pointer to the
     * current carmi index.
     *
     * @param[in] t the pointer of the carmi tree
     */
    explicit inline const_iterator(CARMIMap *t)
        : tree(t), currnode(NULL), currblock(0), currslot(0) {}

    /**
     * @brief Construct a new const iterator object.
     *
     * @param[in] t the pointer of the carmi tree
     * @param[in] node the pointer of the current leaf node
     * @param[in] block the index of the current data block in the leaf node
     * @param[in] slot the index of the data points in the data block
     */
    inline const_iterator(CARMIMap *t,
                          BaseNode<KeyType, ValueType, Compare, Alloc> *node,
                          int block, int slot)
        : tree(t), currnode(node), currblock(block), currslot(slot) {}

    /**
     * @brief Construct a new const iterator object from an iterator.
     *
     * @param[in] it the given iterator
     */
    explicit inline const_iterator(const iterator &it)
        : tree(it.tree),
          currnode(it.currnode),
          currblock(it.currblock),
          currslot(it.currslot) {}

    /**
     * @brief Construct a new const iterator object from a reverse iterator.
     *
     * @param[in] it the given iterator
     */
    explicit inline const_iterator(const reverse_iterator &it)
        : tree(it.tree),
          currnode(it.currnode),
          currblock(it.currblock),
          currslot(it.currslot) {}

    /**
     * @brief Construct a new const iterator object from a const reverse
     * iterator.
     *
     * @param[in] it the given iterator
     */
    explicit inline const_iterator(const const_reverse_iterator &it)
        : tree(it.tree),
          currnode(it.currnode),
          currblock(it.currblock),
          currslot(it.currslot) {}

    /**
     * @brief Get the key value of this iterator.
     *
     * @return const KeyType& the key value
     */
    inline const KeyType &key() const {
      // return the key value in the data block
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot]
          .first;
    }

    /**
     * @brief Get the data value of this iterator.
     *
     * @return const ValueType& the data value
     */
    inline const ValueType &data() const {
      // return the data value in the data block
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot]
          .second;
    }

    /**
     * @brief Return a reference of the data point of this iterator.
     *
     * @return DataType& the reference of this data point
     */
    inline DataType &operator*() const {
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot];
    }

    /**
     * @brief Return the pointer to this data point.
     *
     * @return DataType* the pointer to this data point
     */
    inline DataType *operator->() const {
      return &(
          tree->carmi_tree.data.dataArray[currnode->cfArray.m_left + currblock]
              .slots[currslot]);
    }

    /**
     * @brief Check if the given iterator x is equal to this iterator.
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is equal to this iterator
     * @retval false the given iterator is unequal to this iterator
     */
    inline bool operator==(const const_iterator &x) const {
      return (x.tree == tree && x.currnode == currnode) &&
             (x.currslot == currslot && x.currblock == currblock);
    }

    /**
     * @brief Check if the given iterator x is unequal to this iterator.
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is unequal to this iterator
     * @retval false the given iterator is equal to this iterator
     */
    inline bool operator!=(const const_iterator &x) const {
      return (x.currnode != currnode || x.tree != tree) ||
             (x.currslot != currslot || x.currblock != currblock);
    }

    /**
     * @brief Prefix++ get the iterator of the next data point.
     *
     * @return const_iterator& the next iterator
     */
    inline const_iterator &operator++() {
      // Case 1: the next data point is stored in the same data block,
      // return it
      bool isSuccess = advanceSlot();
      if (isSuccess) {
        return *this;
      }

      // Case 2: the next data point is stored in the next data block
      isSuccess = advanceBlock();
      if (isSuccess) {
        return *this;
      }

      // Case 3: the next data point is stored in the next leaf node,
      // update the variables and continue to find the next data point
      isSuccess = advanceNode();
      if (isSuccess) {
        return *this;
      }
      // Case 4: the next data point is invalid, return end()
      *this = this->tree->cend();
      return *this;
    }

    /**
     * @brief Postfix++ get the iterator of the next data point.
     *
     * @return const_iterator& the next iterator
     */
    inline const_iterator operator++(int) {
      const_iterator tmp = *this;  // copy ourselves
      // Case 1: the next data point is stored in the same data block,
      // return it
      bool isSuccess = advanceSlot();
      if (isSuccess) {
        return tmp;
      }

      // Case 2: the next data point is stored in the next data block
      isSuccess = advanceBlock();
      if (isSuccess) {
        return tmp;
      }

      // Case 3: the next data point is stored in the next leaf node,
      // update the variables and continue to find the next data point
      isSuccess = advanceNode();
      if (isSuccess) {
        return tmp;
      }
      // Case 4: the next data point is invalid, return end()
      *this = this->tree->cend();
      return tmp;
    }

    /**
     * @brief Prefix-- get the iterator of the previous data point.
     *
     * @return const_iterator& the previous iterator
     */
    inline const_iterator &operator--() {
      // Case 1: the previous data point is stored in the same data block,
      // return it
      bool isSuccess = backwardSlot();
      if (isSuccess) {
        return *this;
      }

      // Case 2: the previous data point is stored in the previous data block
      isSuccess = backwardBlock();
      if (isSuccess) {
        return *this;
      }

      // Case 3: the previous data point is stored in the previous leaf node,
      // update the variables and continue to find the previous data point
      isSuccess = backwardNode();
      if (isSuccess) {
        return *this;
      }
      // Case 4: the previous data point is invalid, return end()
      *this = this->tree->cend();
      return *this;
    }

    /**
     * @brief Postfix-- get the iterator of the previous data point.
     *
     * @return const_iterator& the previous iterator
     */
    inline const_iterator operator--(int) {
      const_iterator tmp = *this;  // copy ourselves
      // Case 1: the previous data point is stored in the same data block,
      // return it
      bool isSuccess = backwardSlot();
      if (isSuccess) {
        return tmp;
      }

      // Case 2: the previous data point is stored in the previous data block
      isSuccess = backwardBlock();
      if (isSuccess) {
        return tmp;
      }

      // Case 3: the previous data point is stored in the previous leaf node,
      // update the variables and continue to find the previous data point
      isSuccess = backwardNode();
      if (isSuccess) {
        return tmp;
      }
      // Case 4: the previous data point is invalid, return end()
      *this = this->tree->cend();
      return tmp;
    }

   private:
    //*** Private Functions of Iterator Objects

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the same data
     * block.
     *
     * @retval true point to the next data point
     * @retval false the next data point is not stored in the same data block
     */
    inline bool advanceSlot() {
      currslot++;
      if (currslot < currnode->cfArray.GetBlockSize(currblock)) {
        return true;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the next data
     * block.
     *
     * @retval true point to the next data point
     * @retval false the next data point is not stored in the netx data block
     */
    inline bool advanceBlock() {
      currblock++;
      while (currblock < (currnode->cfArray.flagNumber & 0x00FFFFFF)) {
        currslot = -1;
        bool isSuccess = advanceSlot();
        if (isSuccess) {
          return true;
        }
        currblock++;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the next leaf node.
     *
     * @retval true point to the next data point
     * @retval false the current data point is the last one
     */
    inline bool advanceNode() {
      while (currnode->cfArray.nextLeaf != -1) {
        currnode =
            &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
        while ((currnode->cfArray.flagNumber & 0x00FFFFFF) == 0) {
          if (currnode->cfArray.nextLeaf == -1) {
            return false;
          }
          currnode =
              &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
        }
        currblock = -1;
        bool isSuccess = advanceBlock();
        if (isSuccess) {
          return true;
        }
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the same
     * data block.
     *
     * @retval true point to the previous data point
     * @retval false the previous data point is not stored in the same data
     * block
     */
    inline bool backwardSlot() {
      currslot--;
      if (currslot >= 0) {
        return true;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the
     * previous data block.
     *
     * @retval true point to the previous data point
     * @retval false the previous data point is not stored in the netx data
     * block
     */
    inline bool backwardBlock() {
      currblock--;
      while (currblock >= 0) {
        currslot = currnode->cfArray.GetBlockSize(currblock);
        bool isSuccess = backwardSlot();
        if (isSuccess) {
          return true;
        }
        currblock--;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the
     * previous leaf node.
     *
     * @retval true point to the previous data point
     * @retval false the current data point is the last one
     */
    inline bool backwardNode() {
      while (currnode->cfArray.previousLeaf != -1) {
        currnode =
            &(tree->carmi_tree.node.nodeArray[currnode->cfArray.previousLeaf]);
        while ((currnode->cfArray.flagNumber & 0x00FFFFFF) == 0) {
          if (currnode->cfArray.previousLeaf == -1) {
            return false;
          }
          currnode = &(
              tree->carmi_tree.node.nodeArray[currnode->cfArray.previousLeaf]);
        }
        currblock = (currnode->cfArray.flagNumber & 0x00FFFFFF);
        bool isSuccess = backwardBlock();
        if (isSuccess) {
          return true;
        }
      }
      return false;
    }

   private:
    //*** Private Data Members of Iterator Objects

    /**
     * @brief the pointer of the carmi tree
     */
    CARMIMap *tree;

    /**
     * @brief the pointer of the current leaf node
     */
    BaseNode<KeyType, ValueType, Compare, Alloc> *currnode;

    /**
     * @brief the index of the current data block in the leaf node
     */
    int currblock;

    /**
     * @brief the index of the data point in the data block
     */
    int currslot;

    friend class iterator;
    friend class reverse_iterator;
    friend class const_reverse_iterator;
    friend class CARMIMap;
  };

  /**
   * @brief The reverse iterator of CARMIMap items. The iterator points to a
   * specific data point in the carmi_tree and provides some basic functions as
   * the STL::map.
   */
  class reverse_iterator {
   public:
    /**
     * @brief Construct an empty new reverse iterator object with the default
     * values.
     */
    inline reverse_iterator() {}

    /**
     * @brief Construct a new reverse iterator object and set the pointer to the
     * current carmi index.
     *
     * @param[in] t the pointer of the carmi tree
     */
    explicit inline reverse_iterator(CARMIMap *t)
        : tree(t), currnode(NULL), currblock(0), currslot(0) {}

    /**
     * @brief Construct a new reverse iterator object.
     *
     * @param[in] t the pointer of the carmi tree
     * @param[in] node the pointer of the current leaf node
     * @param[in] block the index of the current data block in the leaf node
     * @param[in] slot the index of the data points in the data block
     */
    inline reverse_iterator(CARMIMap *t,
                            BaseNode<KeyType, ValueType, Compare, Alloc> *node,
                            int block, int slot)
        : tree(t), currnode(node), currblock(block), currslot(slot) {}

    /**
     * @brief Construct a new reverse iterator object from an iterator.
     *
     * @param[in] it the given iterator
     */
    explicit inline reverse_iterator(const iterator &it)
        : tree(it.tree),
          currnode(it.currnode),
          currblock(it.currblock),
          currslot(it.currslot) {}

    /**
     * @brief Construct a new reverse iterator object from a reverse iterator.
     *
     * @param[in] it the given iterator
     */
    explicit inline reverse_iterator(const reverse_iterator &it)
        : tree(it.tree),
          currnode(it.currnode),
          currblock(it.currblock),
          currslot(it.currslot) {}

    /**
     * @brief Get the key value of this iterator.
     *
     * @return const KeyType& the key value
     */
    inline const KeyType &key() const {
      // return the key value in the data block
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot - 1]
          .first;
    }

    /**
     * @brief Get the data value of this iterator.
     *
     * @return const ValueType& the data value
     */
    inline const ValueType &data() const {
      // return the data value in the data block
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot - 1]
          .second;
    }

    /**
     * @brief Return a reference of the data point of this iterator.
     *
     * @return DataType& the reference of this data point
     */
    inline DataType &operator*() const {
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot - 1];
    }

    /**
     * @brief Return the pointer to this data point.
     *
     * @return DataType* the pointer to this data point
     */
    inline DataType *operator->() const {
      return &(
          tree->carmi_tree.data.dataArray[currnode->cfArray.m_left + currblock]
              .slots[currslot - 1]);
    }

    /**
     * @brief Check if the given iterator x is equal to this iterator.
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is equal to this iterator
     * @retval false the given iterator is unequal to this iterator
     */
    inline bool operator==(const reverse_iterator &x) const {
      return (x.tree == tree && x.currnode == currnode) &&
             (x.currslot == currslot && x.currblock == currblock);
    }

    /**
     * @brief Check if the given iterator x is unequal to this iterator.
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is unequal to this iterator
     * @retval false the given iterator is equal to this iterator
     */
    inline bool operator!=(const reverse_iterator &x) const {
      return (x.currnode != currnode || x.tree != tree) ||
             (x.currslot != currslot || x.currblock != currblock);
    }

    /**
     * @brief Prefix++ get the iterator of the next data point.
     *
     * @return reverse_iterator& the next iterator
     */
    inline reverse_iterator &operator++() {
      // Case 1: the next data point is stored in the same data block,
      // return it
      bool isSuccess = advanceSlot();
      if (isSuccess) {
        return *this;
      }

      // Case 2: the next data point is stored in the next data block
      isSuccess = advanceBlock();
      if (isSuccess) {
        return *this;
      }

      // Case 3: the next data point is stored in the next leaf node,
      // update the variables and continue to find the next data point
      isSuccess = advanceNode();
      if (isSuccess) {
        return *this;
      }
      // Case 4: the next data point is invalid, return end()
      *this = this->tree->rend();
      return *this;
    }

    /**
     * @brief Postfix++ get the iterator of the next data point.
     *
     * @return reverse_iterator& the next iterator
     */
    inline reverse_iterator operator++(int) {
      reverse_iterator tmp = *this;  // copy ourselves
      // Case 1: the next data point is stored in the same data block,
      // return it
      bool isSuccess = advanceSlot();
      if (isSuccess) {
        return tmp;
      }

      // Case 2: the next data point is stored in the next data block
      isSuccess = advanceBlock();
      if (isSuccess) {
        return tmp;
      }

      // Case 3: the next data point is stored in the next leaf node,
      // update the variables and continue to find the next data point
      isSuccess = advanceNode();
      if (isSuccess) {
        return tmp;
      }
      // Case 4: the next data point is invalid, return end()
      *this = this->tree->rend();
      return tmp;
    }

    /**
     * @brief Prefix-- get the iterator of the previous data point.
     *
     * @return reverse_iterator& the previous iterator
     */
    inline reverse_iterator &operator--() {
      // Case 1: the previous data point is stored in the same data block,
      // return it
      bool isSuccess = backwardSlot();
      if (isSuccess) {
        return *this;
      }

      // Case 2: the previous data point is stored in the previous data block
      isSuccess = backwardBlock();
      if (isSuccess) {
        return *this;
      }

      // Case 3: the previous data point is stored in the previous leaf node,
      // update the variables and continue to find the previous data point
      isSuccess = backwardNode();
      if (isSuccess) {
        return *this;
      }
      // Case 4: the previous data point is invalid, return end()
      *this = this->tree->rend();
      return *this;
    }

    /**
     * @brief Postfix-- get the iterator of the previous data point.
     *
     * @return reverse_iterator& the previous iterator
     */
    inline reverse_iterator operator--(int) {
      reverse_iterator tmp = *this;  // copy ourselves
      // Case 1: the previous data point is stored in the same data block,
      // return it
      bool isSuccess = backwardSlot();
      if (isSuccess) {
        return tmp;
      }

      // Case 2: the previous data point is stored in the previous data block
      isSuccess = backwardBlock();
      if (isSuccess) {
        return tmp;
      }

      // Case 3: the previous data point is stored in the previous leaf node,
      // update the variables and continue to find the previous data point
      isSuccess = backwardNode();
      if (isSuccess) {
        return tmp;
      }
      // Case 4: the previous data point is invalid, return end()
      *this = this->tree->rend();
      return tmp;
    }

   private:
    //*** Private Functions of Iterator Objects

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the same data
     * block.
     *
     * @retval true point to the next data point
     * @retval false the next data point is not stored in the same data block
     */
    inline bool advanceSlot() {
      currslot--;
      if (currslot > 0) {
        return true;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the next data
     * block.
     *
     * @retval true point to the next data point
     * @retval false the next data point is not stored in the netx data block
     */
    inline bool advanceBlock() {
      currblock--;
      while (currblock > 0) {
        currslot = currnode->cfArray.GetBlockSize(currblock) + 1;
        bool isSuccess = backwardSlot();
        if (isSuccess) {
          return true;
        }
        currblock--;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the next leaf node.
     *
     * @retval true point to the next data point
     * @retval false the current data point is the last one
     */
    inline bool advanceNode() {
      while (currnode->cfArray.previousLeaf != -1) {
        currnode =
            &(tree->carmi_tree.node.nodeArray[currnode->cfArray.previousLeaf]);
        while ((currnode->cfArray.flagNumber & 0x00FFFFFF) == 0) {
          if (currnode->cfArray.previousLeaf == -1) {
            return false;
          }
          currnode = &(
              tree->carmi_tree.node.nodeArray[currnode->cfArray.previousLeaf]);
        }
        currblock = (currnode->cfArray.flagNumber & 0x00FFFFFF);
        bool isSuccess = backwardBlock();
        if (isSuccess) {
          return true;
        }
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the same
     * data block.
     *
     * @retval true point to the previous data point
     * @retval false the previous data point is not stored in the same data
     * block
     */
    inline bool backwardSlot() {
      currslot++;
      if (currslot <= currnode->cfArray.GetBlockSize(currblock)) {
        return true;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the
     * previous data block.
     *
     * @retval true point to the previous data point
     * @retval false the previous data point is not stored in the netx data
     * block
     */
    inline bool backwardBlock() {
      currblock++;
      while (currblock <= (currnode->cfArray.flagNumber & 0x00FFFFFF)) {
        currslot = 0;
        bool isSuccess = backwardSlot();
        if (isSuccess) {
          return true;
        }
        currblock++;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the
     * previous leaf node.
     *
     * @retval true point to the previous data point
     * @retval false the current data point is the last one
     */
    inline bool backwardNode() {
      while (currnode->cfArray.nextLeaf != -1) {
        currnode =
            &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
        while ((currnode->cfArray.flagNumber & 0x00FFFFFF) == 0) {
          if (currnode->cfArray.nextLeaf == -1) {
            return false;
          }
          currnode =
              &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
        }
        currblock = -1;
        bool isSuccess = backwardBlock();
        if (isSuccess) {
          return true;
        }
      }
      return false;
    }

   private:
    //*** Private Data Members of Iterator Objects

    /**
     * @brief the pointer of the carmi tree
     */
    CARMIMap *tree;

    /**
     * @brief the pointer of the current leaf node
     */
    BaseNode<KeyType, ValueType, Compare, Alloc> *currnode;

    /**
     * @brief the index of the current data block in the leaf node
     */
    int currblock;

    /**
     * @brief the index of the data point in the data block
     */
    int currslot;

    friend class iterator;
    friend class const_iterator;
    friend class const_reverse_iterator;
    friend class CARMIMap;
  };

  /**
   * @brief The const reverse iterator of CARMIMap items. The iterator points to
   * a specific data point in the carmi_tree and provides some basic functions
   * as the STL::map.
   */
  class const_reverse_iterator {
   public:
    /**
     * @brief Construct an empty new const reverse iterator object with the
     * default values.
     */
    inline const_reverse_iterator() {}

    /**
     * @brief Construct a new const reverse iterator object and set the pointer
     * to the current carmi index.
     *
     * @param[in] t the pointer of the carmi tree
     */
    explicit inline const_reverse_iterator(CARMIMap *t)
        : tree(t), currnode(NULL), currblock(0), currslot(0) {}

    /**
     * @brief Construct a new reverse iterator object.
     *
     * @param[in] t the pointer of the carmi tree
     * @param[in] node the pointer of the current leaf node
     * @param[in] block the index of the current data block in the leaf node
     * @param[in] slot the index of the data points in the data block
     */
    inline const_reverse_iterator(
        CARMIMap *t, BaseNode<KeyType, ValueType, Compare, Alloc> *node,
        int block, int slot)
        : tree(t), currnode(node), currblock(block), currslot(slot) {}

    /**
     * @brief Construct a new reverse iterator object from an iterator.
     *
     * @param[in] it the given iterator
     */
    explicit inline const_reverse_iterator(const iterator &it)
        : tree(it.tree),
          currnode(it.currnode),
          currblock(it.currblock),
          currslot(it.currslot) {}

    /**
     * @brief Construct a new reverse iterator object from a const iterator.
     *
     * @param[in] it the given iterator
     */
    explicit inline const_reverse_iterator(const const_iterator &it)
        : tree(it.tree),
          currnode(it.currnode),
          currblock(it.currblock),
          currslot(it.currslot) {}

    /**
     * @brief Construct a new reverse iterator object from a reverse iterator.
     *
     * @param[in] it the given iterator
     */
    explicit inline const_reverse_iterator(const reverse_iterator &it)
        : tree(it.tree),
          currnode(it.currnode),
          currblock(it.currblock),
          currslot(it.currslot) {}

    /**
     * @brief Get the key value of this iterator.
     *
     * @return const KeyType& the key value
     */
    inline const KeyType &key() const {
      // return the key value in the data block
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot - 1]
          .first;
    }

    /**
     * @brief Get the data value of this iterator.
     *
     * @return const ValueType& the data value
     */
    inline const ValueType &data() const {
      // return the data value in the data block
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot - 1]
          .second;
    }

    /**
     * @brief Return a reference of the data point of this iterator.
     *
     * @return DataType& the reference of this data point
     */
    inline DataType &operator*() const {
      return tree->carmi_tree.data
          .dataArray[currnode->cfArray.m_left + currblock]
          .slots[currslot - 1];
    }

    /**
     * @brief Return the pointer to this data point.
     *
     * @return DataType* the pointer to this data point
     */
    inline DataType *operator->() const {
      return &(
          tree->carmi_tree.data.dataArray[currnode->cfArray.m_left + currblock]
              .slots[currslot - 1]);
    }

    /**
     * @brief Check if the given iterator x is equal to this iterator.
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is equal to this iterator
     * @retval false the given iterator is unequal to this iterator
     */
    inline bool operator==(const const_reverse_iterator &x) const {
      return (x.tree == tree && x.currnode == currnode) &&
             (x.currslot == currslot && x.currblock == currblock);
    }

    /**
     * @brief Check if the given iterator x is unequal to this iterator.
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is unequal to this iterator
     * @retval false the given iterator is equal to this iterator
     */
    inline bool operator!=(const const_reverse_iterator &x) const {
      return (x.currnode != currnode || x.tree != tree) ||
             (x.currslot != currslot || x.currblock != currblock);
    }

    /**
     * @brief Prefix++ get the iterator of the next data point.
     *
     * @return const_reverse_iterator& the next iterator
     */
    inline const_reverse_iterator &operator++() {
      // Case 1: the next data point is stored in the same data block,
      // return it
      bool isSuccess = advanceSlot();
      if (isSuccess) {
        return *this;
      }

      // Case 2: the next data point is stored in the next data block
      isSuccess = advanceBlock();
      if (isSuccess) {
        return *this;
      }

      // Case 3: the next data point is stored in the next leaf node,
      // update the variables and continue to find the next data point
      isSuccess = advanceNode();
      if (isSuccess) {
        return *this;
      }
      // Case 4: the next data point is invalid, return end()
      *this = this->tree->crend();
      return *this;
    }

    /**
     * @brief Postfix++ get the iterator of the next data point.
     *
     * @return const_reverse_iterator& the next iterator
     */
    inline const_reverse_iterator operator++(int) {
      const_reverse_iterator tmp = *this;  // copy ourselves
      // Case 1: the next data point is stored in the same data block,
      // return it
      bool isSuccess = advanceSlot();
      if (isSuccess) {
        return tmp;
      }

      // Case 2: the next data point is stored in the next data block
      isSuccess = advanceBlock();
      if (isSuccess) {
        return tmp;
      }

      // Case 3: the next data point is stored in the next leaf node,
      // update the variables and continue to find the next data point
      isSuccess = advanceNode();
      if (isSuccess) {
        return tmp;
      }
      // Case 4: the next data point is invalid, return end()
      *this = this->tree->crend();
      return tmp;
    }

    /**
     * @brief Prefix-- get the iterator of the previous data point.
     *
     * @return const_reverse_iterator& the previous iterator
     */
    inline const_reverse_iterator &operator--() {
      // Case 1: the previous data point is stored in the same data block,
      // return it
      bool isSuccess = backwardSlot();
      if (isSuccess) {
        return *this;
      }

      // Case 2: the previous data point is stored in the previous data block
      isSuccess = backwardBlock();
      if (isSuccess) {
        return *this;
      }

      // Case 3: the previous data point is stored in the previous leaf node,
      // update the variables and continue to find the previous data point
      isSuccess = backwardNode();
      if (isSuccess) {
        return *this;
      }
      // Case 4: the previous data point is invalid, return end()
      *this = this->tree->crend();
      return *this;
    }

    /**
     * @brief Postfix-- get the iterator of the previous data point.
     *
     * @return const_reverse_iterator& the previous iterator
     */
    inline const_reverse_iterator operator--(int) {
      const_reverse_iterator tmp = *this;  // copy ourselves
      // Case 1: the previous data point is stored in the same data block,
      // return it
      bool isSuccess = backwardSlot();
      if (isSuccess) {
        return tmp;
      }

      // Case 2: the previous data point is stored in the previous data block
      isSuccess = backwardBlock();
      if (isSuccess) {
        return tmp;
      }

      // Case 3: the previous data point is stored in the previous leaf node,
      // update the variables and continue to find the previous data point
      isSuccess = backwardNode();
      if (isSuccess) {
        return tmp;
      }
      // Case 4: the previous data point is invalid, return end()
      *this = this->tree->crend();
      return tmp;
    }

   private:
    //*** Private Functions of Iterator Objects

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the same data
     * block.
     *
     * @retval true point to the next data point
     * @retval false the next data point is not stored in the same data block
     */
    inline bool advanceSlot() {
      currslot--;
      if (currslot > 0) {
        return true;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the next data
     * block.
     *
     * @retval true point to the next data point
     * @retval false the next data point is not stored in the netx data block
     */
    inline bool advanceBlock() {
      currblock--;
      while (currblock > 0) {
        currslot = currnode->cfArray.GetBlockSize(currblock) + 1;
        bool isSuccess = backwardSlot();
        if (isSuccess) {
          return true;
        }
        currblock--;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the next location and point to the
     * next data point, and the next data point is store in the next leaf node.
     *
     * @retval true point to the next data point
     * @retval false the current data point is the last one
     */
    inline bool advanceNode() {
      while (currnode->cfArray.previousLeaf != -1) {
        currnode =
            &(tree->carmi_tree.node.nodeArray[currnode->cfArray.previousLeaf]);
        while ((currnode->cfArray.flagNumber & 0x00FFFFFF) == 0) {
          if (currnode->cfArray.previousLeaf == -1) {
            return false;
          }
          currnode = &(
              tree->carmi_tree.node.nodeArray[currnode->cfArray.previousLeaf]);
        }
        currblock = (currnode->cfArray.flagNumber & 0x00FFFFFF);
        bool isSuccess = backwardBlock();
        if (isSuccess) {
          return true;
        }
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the same
     * data block.
     *
     * @retval true point to the previous data point
     * @retval false the previous data point is not stored in the same data
     * block
     */
    inline bool backwardSlot() {
      currslot++;
      if (currslot <= currnode->cfArray.GetBlockSize(currblock)) {
        return true;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the
     * previous data block.
     *
     * @retval true point to the previous data point
     * @retval false the previous data point is not stored in the netx data
     * block
     */
    inline bool backwardBlock() {
      currblock++;
      while (currblock <= (currnode->cfArray.flagNumber & 0x00FFFFFF)) {
        currslot = 0;
        bool isSuccess = backwardSlot();
        if (isSuccess) {
          return true;
        }
        currblock++;
      }
      return false;
    }

    /**
     * @brief Move the current iterator to the previous location and point to
     * the previous data point, and the previous data point is store in the
     * previous leaf node.
     *
     * @retval true point to the previous data point
     * @retval false the current data point is the last one
     */
    inline bool backwardNode() {
      while (currnode->cfArray.nextLeaf != -1) {
        currnode =
            &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
        while ((currnode->cfArray.flagNumber & 0x00FFFFFF) == 0) {
          if (currnode->cfArray.nextLeaf == -1) {
            return false;
          }
          currnode =
              &(tree->carmi_tree.node.nodeArray[currnode->cfArray.nextLeaf]);
        }
        currblock = -1;
        bool isSuccess = backwardBlock();
        if (isSuccess) {
          return true;
        }
      }
      return false;
    }

   private:
    //*** Private Data Members of Iterator Objects

    /**
     * @brief the pointer of the carmi tree
     */
    CARMIMap *tree;

    /**
     * @brief the pointer of the current leaf node
     */
    BaseNode<KeyType, ValueType, Compare, Alloc> *currnode;

    /**
     * @brief the index of the current data block in the leaf node
     */
    int currblock;

    /**
     * @brief the index of the data point in the data block
     */
    int currslot;

    friend class iterator;
    friend class const_iterator;
    friend class reverse_iterator;
    friend class CARMIMap;
  };

 private:
  //*** The Private Functions of Constructor

  /**
   * @brief Preprocess the input dataset between the first iterator and the
   * last iterator and construct the dataset in the form of vector.
   *
   * @tparam InputIterator the iterator type of input
   * @param[in] first the first input iterator
   * @param[in] last the last input iterator
   * @param[out] dataset the constructed dataset: [{key_0, value_0}, {key_1,
   * value_1}, ..., {key_n, value_n}].
   */
  template <typename InputIterator>
  void PreprocessInput(const InputIterator &first, const InputIterator &last,
                       DataVectorType *dataset) {
    InputIterator iter = first;
    while (iter != last) {
      dataset->push_back(*iter);
      ++iter;
    }
    std::sort(dataset->begin(), dataset->end(),
              [this](const auto &a, const auto &b) {
                return carmi_tree.key_less_(a.first, b.first);
              });
  }

  /**
   * @brief Construct the findQuery and insertQuery for the cost model and
   * call the hybrid construction algorithm to initialize the carmi_tree and
   * construct the optimal CARMI index structure.
   *
   * @param[in] initDataset the init dataset: [{key_0, value_0}, {key_1,
   * value_1}, ..., {key_n, value_n}].
   * @param[in] insertDataset the insert dataset: [{key_0, value_0}, {key_1,
   * value_1}, ..., {key_n, value_n}].
   * @param[in] lambda lambda (time + lambda * space)
   */
  void Init(const DataVectorType &initDataset,
            const DataVectorType &insertDataset, double lambda) {
    QueryType findQuery(initDataset.size());
    KeyVectorType insertQuery(insertDataset.size());

    // set the frequency, each data point is accessed once in the historical
    // find queries
    for (int i = 0; i < static_cast<int>(findQuery.size()); i++) {
      findQuery[i].first = initDataset[i].first;
      findQuery[i].second = 1;
    }
    for (int i = 0; i < static_cast<int>(insertQuery.size()); i++) {
      insertQuery[i] = insertDataset[i].first;
    }

    // construct carmi
    carmi_tree = carmi_impl(initDataset, findQuery, insertQuery, lambda);
    carmi_tree.Construction();
  }

 public:
  // *** Basic Functions of CARMI Common Objects

 public:
  // *** STL Iterator Construction Functions

  /**
   * @brief Returns an iterator referring to the first element in the carmi
   * tree.
   *
   * @return iterator
   */
  iterator begin() {
    iterator it(this);
    if (it.tree == NULL || carmi_tree.firstLeaf < 0 ||
        carmi_tree.firstLeaf >= carmi_tree.node.nodeArray.size()) {
      throw std::invalid_argument(
          "CARMIMap::begin: the index has not been constructed.");
    }
    it.currnode = &carmi_tree.node.nodeArray[carmi_tree.firstLeaf];
    while (it.currnode->cfArray.GetBlockSize(it.currblock) == 0) {
      it++;
    }
    return it;
  }

  /**
   * @brief Get the end of the carmi tree.
   *
   * @return iterator
   */
  iterator end() { return iterator(this); }

  /**
   * @brief Returns a const iterator referring to the first element in the carmi
   * tree.
   *
   * @return const_iterator
   */
  const_iterator cbegin() const {
    const_iterator it(this);
    if (it.tree == NULL || carmi_tree.firstLeaf < 0 ||
        carmi_tree.firstLeaf >= carmi_tree.node.nodeArray.size()) {
      throw std::invalid_argument(
          "CARMIMap::cbegin: the index has not been constructed.");
    }
    it.currnode = &carmi_tree.node.nodeArray[carmi_tree.firstLeaf];
    while (it.currnode->cfArray.GetBlockSize(it.currblock) == 0) {
      it++;
    }
    return it;
  }

  /**
   * @brief Get the end of the carmi tree.
   *
   * @return const_iterator
   */
  const_iterator cend() const { return const_iterator(this); }

  /**
   * @brief Returns a reverse iterator referring to the first element in the
   * carmi tree.
   *
   * @return reverse_iterator
   */
  reverse_iterator rbegin() {
    reverse_iterator it(this);
    if (it.tree == NULL || carmi_tree.lastLeaf < 0 ||
        carmi_tree.lastLeaf >= carmi_tree.node.nodeArray.size()) {
      throw std::invalid_argument(
          "CARMIMap::rbegin: the index has not been constructed.");
    }
    it.currnode = &carmi_tree.node.nodeArray[carmi_tree.lastLeaf];
    it.currblock = (it.currnode->cfArray.flagNumber & 0x00FFFFFF) - 1;
    it.currslot = it.currnode->cfArray.perSize[it.currblock];
    while (it.currnode->cfArray.GetBlockSize(it.currblock) == 0) {
      it++;
    }
    return it;
  }

  /**
   * @brief Get the end of the carmi tree.
   *
   * @return reverse_iterator
   */
  reverse_iterator rend() { return reverse_iterator(this); }

  /**
   * @brief Returns a reverse iterator referring to the first element in the
   * carmi tree.
   *
   * @return const_reverse_iterator
   */
  const_reverse_iterator crbegin() const {
    const_reverse_iterator it(this);
    if (it.tree == NULL || carmi_tree.lastLeaf < 0 ||
        carmi_tree.lastLeaf >= carmi_tree.node.nodeArray.size()) {
      throw std::invalid_argument(
          "CARMIMap::crbegin: the index has not been constructed.");
    }
    it.currnode = &carmi_tree.node.nodeArray[carmi_tree.lastLeaf];
    it.currblock = (it.currnode->cfArray.flagNumber & 0x00FFFFFF) - 1;
    it.currslot = it.currnode->cfArray.perSize[it.currblock];
    while (it.currnode->cfArray.GetBlockSize(it.currblock) == 0) {
      it++;
    }
    return it;
  }

  /**
   * @brief Get the end of the carmi tree.
   *
   * @return const_reverse_iterator
   */
  const_reverse_iterator crend() const { return const_reverse_iterator(this); }

 public:
  // *** Standard Access Functions Querying the Tree

  /**
   * @brief Returns an iterator pointing to the first element in the container
   * whose key is not less than key.
   *
   * @param[in] key the given key value
   * @return iterator the iterator to the first element whose key is not less
   * than key
   */
  iterator lower_bound(const KeyType &key) {
    iterator it(this);
    it.currnode = carmi_tree.Find(key, &it.currblock, &it.currslot);
    if (it.currslot >= it.currnode->cfArray.GetBlockSize(it.currblock)) {
      it++;
    }
    return it;
  }

  /**
   * @brief Returns an iterator pointing to the first element in the container
   * whose key is not less than key.
   *
   * @param[in] key the given key value
   * @return const_iterator the iterator to the first element whose key is not
   * less than key
   */
  const_iterator lower_bound(const KeyType &key) const {
    const_iterator it(this);
    it.currnode = carmi_tree.Find(key, &it.currblock, &it.currslot);
    if (it.currslot >= it.currnode->cfArray.GetBlockSize(it.currblock)) {
      it++;
    }
    return it;
  }

  /**
   * @brief Returns an iterator pointing to the first element in the container
   * whose key is greater than key.
   *
   * @param[in] key the given key value
   * @return iterator the iterator to the first element whose key is greater
   * than key
   */
  iterator upper_bound(const KeyType &key) {
    iterator it(this);
    it.currnode = carmi_tree.Find(key, &it.currblock, &it.currslot);
    while (it != end() && it.key() == key) {
      it++;
    }
    return it;
  }

  /**
   * @brief Returns an iterator pointing to the first element in the container
   * whose key is greater than key.
   *
   * @param[in] key the given key value
   * @return const_iterator the iterator to the first element whose key is
   * greater than key
   */
  const_iterator upper_bound(const KeyType &key) const {
    const_iterator it(this);
    it.currnode = carmi_tree.Find(key, &it.currblock, &it.currslot);
    while (it != cend() && it.key() == key) {
      it++;
    }
    return it;
  }

  /**
   * @brief Finds the corresponding iterator of the given key value.
   *
   * @param[in] key the given key value
   * @return iterator the iterator of the data point
   */
  iterator find(const KeyType &key) {
    int currblock, currslot;
    BaseNode<KeyType, ValueType, Compare, Alloc> *node =
        carmi_tree.Find(key, &currblock, &currslot);
    if (currslot >= node->cfArray.GetBlockSize(currblock)) {
      return end();
    } else {
      return iterator(this, node, currblock, currslot);
    }
  }

  /**
   * @brief Finds the corresponding iterator of the given key value.
   *
   * @param[in] key the given key value
   * @return const_iterator the iterator of the data point
   */
  const_iterator find(const KeyType &key) const {
    int currblock, currslot;
    BaseNode<KeyType, ValueType, Compare, Alloc> *node =
        carmi_tree.Find(key, &currblock, &currslot);
    if (currslot >= node->cfArray.GetBlockSize(currblock)) {
      return cend();
    } else {
      return const_iterator(this, node, currblock, currslot);
    }
  }

  /**
   * @brief Returns the bounds of a range that includes all the elements in the
   * container which have a key equivalent to the given key value.
   *
   * @param[in] key the given key value
   * @return std::pair<iterator, iterator> [lower_bound, upper_bound)
   */
  inline std::pair<iterator, iterator> equal_range(const KeyType &key) {
    return std::pair<iterator, iterator>(lower_bound(key), upper_bound(key));
  }

  /**
   * @brief Returns the bounds of a range that includes all the elements in the
   * container which have a key equivalent to the given key value.
   *
   * @param[in] key the given key value
   * @return std::pair<iterator, iterator> [lower_bound, upper_bound)
   */
  inline std::pair<const_iterator, const_iterator> equal_range(
      const KeyType &key) const {
    return std::pair<const_iterator, const_iterator>(lower_bound(key),
                                                     upper_bound(key));
  }

  /**
   * @brief Searches the container for elements with a key equivalent to the
   * given key and returns the number of matches.
   *
   * @param[in] key the given key value
   * @return size_t the number of matched data points
   */
  size_t count(const KeyType &key) {
    iterator it = lower_bound(key);
    size_t num_equal = 0;
    while (it != end() && it.key() == key) {
      num_equal++;
      ++it;
    }
    return num_equal;
  }

 public:
  //*** Public Modify Functions

  /**
   * @brief Inserts the given data point into carmi.
   *
   * @param[in] datapoint the inserted data point
   * @return std::pair<iterator, bool> a pair, with its member pair::first set
   * to an iterator pointing to either the newly inserted element in the map.
   * The pair::second element in the pair is set to true if a new element was
   * inserted.
   */
  std::pair<iterator, bool> insert(const DataType &datapoint) {
    iterator it(this);
    auto res = carmi_tree.Insert(datapoint, &it.currblock, &it.currslot);
    it.currnode = res.first;
    return {it, res.second};
  }

  /**
   * @brief Inserts the given data point into carmi.
   *
   * @param[in] position useless here
   * @param[in] datapoint the inserted data point
   * @return std::pair<iterator, bool> a pair, with its member pair::first set
   * to an iterator pointing to either the newly inserted element in the map.
   * The pair::second element in the pair is set to true if a new element was
   * inserted.
   */
  iterator insert(const_iterator position, const DataType &datapoint) {
    iterator it(this);
    auto res = carmi_tree.Insert(datapoint, &it.currblock, &it.currslot);
    it.currnode = res.first;
    return it;
  }

  /**
   * @brief Copies of the elements in the range [first,last) are inserted in the
   * container.
   *
   * @tparam InputIterator an input iterator type that points to elements of a
   * type from which value_type objects can be constructed.
   * @param[in] first the first iterator
   * @param[in] last the last iterator
   */
  template <class InputIterator>
  void insert(InputIterator first, InputIterator last) {
    for (auto it = first; it != last; ++it) {
      insert(*it);
    }
  }

 public:
  // *** Public Erase Functions

  /**
   * @brief Erases the element at the position of the given iterator.
   *
   * @param[in] it Iterator pointing to a single element to be removed from the
   * index.
   */
  void erase(iterator it) {
    carmi_tree.DeleteSingleData(it.key(), *(it.currnode), it.currblock,
                                it.currslot);
  }

  /**
   * @brief Erases all the data points of the given key.
   *
   * @param[in] key the key value of the deleted record
   * @return the number of elements erased
   */
  size_t erase(const KeyType &key) {
    size_t cnt = 0;
    carmi_tree.Delete(key, &cnt);
    return cnt;
  }

  /**
   * @brief Erases all the data points in the range [first, last).
   *
   * @param[in] first the first iterator
   * @param[in] last the last iterator
   */
  void erase(iterator first, iterator last) {
    for (auto it = first; it != last; ++it) {
      erase(*it);
    }
  }

 public:
  //*** Other Public Modify Functions

  /**
   * @brief Fast swapping of two carmi tree objects.
   *
   * @param[in] other the other carmi tree
   */
  void swap(CARMIMap &other) { std::swap(carmi_tree, other.carmi_tree); }

  /**
   * @brief Removes all elements.
   */
  void clear() { carmi_tree.clear(); }

 public:
  //*** Public Element Access Functions

  /**
   * @brief Inserts a new element with that key and returns a reference to its
   * value.
   *
   * @param[in] key the given key value
   * @return ValueType& the value of it
   */
  ValueType &operator[](const KeyType &key) {
    iterator it(this);
    auto res = carmi_tree.Insert({key, KeyType()}, &it.currblock, &it.currslot);
    it.currnode = res.first;
    return it.data();
  }

  /**
   * @brief Returns a reference to the mapped value of the element identified
   * with key.
   *
   * @param[in] key the given key value
   * @return ValueType& the value of it
   */
  ValueType &at(const KeyType &key) {
    iterator it = find(key);
    if (it == end()) {
      throw std::out_of_range("CARMIMap::at: input does not match any key.");
    } else {
      return it.data();
    }
  }

  /**
   * @brief Returns a reference to the mapped value of the element identified
   * with key.
   *
   * @param[in] key the given key value
   * @return ValueType& the value of it
   */
  const ValueType &at(const KeyType &key) const {
    const_iterator it = find(key);
    if (it == cend()) {
      throw std::out_of_range("CARMIMap::at: input does not match any key.");
    } else {
      return it.data();
    }
  }

 public:
  // *** Access Functions to the Item Count

  /**
   * @brief Returns the number of data points in the carmi_tree.
   *
   * @return size_type the number of data points
   */
  inline size_t size() const { return carmi_tree.currsize; }

  /**
   * @brief Returns true if there is at least one data point in the carmi tree.
   *
   * @retval true there is at least one data point in the carmi tree
   * @retval false there is no data point in the carmi tree
   */
  inline bool empty() const { return (size() == size_t(0)); }

  /**
   * @brief Returns the largest possible size of the CARMI Tree. This is just a
   * function required by the STL standard, the CARMI Tree can hold more items.
   *
   * @return size_t
   */
  inline size_t max_size() const { return INT32_MAX; }

  /**
   * @brief Calculates the space of carmi in bytes.
   *
   * @return long long: space
   */
  long long CalculateSpace() const { return carmi_tree.CalculateSpace(); }

  /**
   * @brief Gets the information of the tree node, return the type identifier
   * of this node, the number of its child nodes and the starting index of the
   * first child node in the node array.
   *
   * @param[in] idx the index of the node in the node array
   * @param[out] childNumber the number of the child nodes of this node
   * @param[out] childStartIndex the starting index of the first child node
   * @return int the type identifier of this node
   */
  int GetNodeInfo(int idx, int *childNumber, int *childStartIndex) {
    if (idx < 0 || idx >= carmi_tree.node.nodeArray.size()) {
      throw std::out_of_range(
          "CARMIMap::GetNodeInfo: input does not match any node.");
    }
    return carmi_tree.GetNodeInfo(idx, childNumber, childStartIndex);
  }

 public:
  // *** Key and Value Comparison Function Objects

  /**
   * @brief Gets the allocator object.
   *
   * @return Alloc
   */
  Alloc get_allocator() const { return carmi_tree.get_allocator(); }

  /**
   * @brief Returns a copy of the comparison object used by the container
   * to compare keys.
   *
   * @return Compare
   */
  Compare key_comp() const { return carmi_tree.key_less_; }

  /**
   * @brief Returns a comparison object that can be used to compare two
   * elements to get whether the key of the first one goes before the
   * second.
   *
   * @return value_compare The comparison object for element values.
   */
  typename CARMI<KeyType, ValueType, Compare, Alloc>::value_compare value_comp()
      const {
    return carmi_tree.value_comp();
  }
};

#endif  // CARMI_MAP_H_
