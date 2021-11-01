/**
 * @file carmi_external_map.h
 * @author Jiaoyi
 * @brief Implements the STL map using the external CARMI tree.
 * @version 3.0
 * @date 2021-04-01
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CARMI_EXTERNAL_MAP_H_
#define CARMI_EXTERNAL_MAP_H_

#include <functional>
#include <memory>
#include <utility>
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
 * @brief The specific external version of the CARMI class template. External
 * CARMI only stores the pointer of the position of external records in the
 * index structure.
 *
 * The implementation of the external CARMI. This class is designed for primary
 * index structures, where the original records are already sorted according
 * to the key value and stored in an external location. In such a case, we only
 * need to store the pointer to external locations.
 *
 * This class provides users with basic operations such as find, insert, update,
 * and delete and so on. Users can automatically obtain the optimal indexes with
 * good performance for a given dataset without the need of manual tuning. In
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
 * @tparam KeyType the type of the given key value
 */
template <typename KeyType, typename Compare = std::less<KeyType>,
          typename Alloc = std::allocator<KeyType>>
class CARMIExternalMap {
 public:
  // *** Constructed Types

  /**
   * @brief the type of implementation of CARMI
   */
  typedef CARMI<KeyType, KeyType> carmi_impl;

  /**
   * @brief the type of the key value vector: [key_0, key_1, ..., key_n].
   */
  typedef std::vector<KeyType> KeyVectorType;

  /**
   * @brief the type of the key value vector: [key, [member_0, ..., member_n]].
   */
  typedef std::pair<KeyType, KeyVectorType> DataType;

 private:
  /**
   * @brief The contained implementation object of CARMI
   */
  carmi_impl carmi_tree;

  Compare key_less_ = Compare();
  Alloc allocator_ = Alloc();

 public:
  // *** Constructors and Destructor

  /**
   * @brief Construct a new empty CARMIExternalMap object
   */
  CARMIExternalMap() : carmi_tree() {}

  /**
   * @brief Destroy the CARMIExternalMap object
   */
  ~CARMIExternalMap() {}

  /**
   * @brief Construct a new empty CARMIExternalMap object with the standard key
   * comparison function
   *
   * @param[in] alloc The allocator object used to define the storage allocation
   * model.
   */
  explicit CARMIExternalMap(const Alloc &alloc) : allocator_(alloc) {}

  /**
   * @brief Construct a new empty CARMIExternalMap object with a special key
   * comparision object.
   *
   * @param[in] comp A binary predicate that takes two element keys as arguments
   * and returns a bool.
   * @param[in] alloc The allocator object used to define the storage allocation
   * model.
   */
  explicit CARMIExternalMap(const Compare &comp = Compare(),
                            const Alloc &alloc = Alloc())
      : key_less_(comp), allocator_(alloc) {}

  /**
   * @brief Construct a new CARMIExternalMap object.
   *
   * The CARMI index structure can achieve a good performance under the
   * time/space setting based on the lambda parameter. The goal of the hybrid
   * construction algorithm is to minimize the total cost of historical access
   * and insert queries, which is: the average time cost of each query + lambda
   * times the space cost of the index tree.
   *
   * @param[in] dataset the pointer to the external dataset used to construct
   * the index and the historical find queries
   * @param[in] record_number the number of the records
   * @param[in] record_len the length of a record (byte)
   * @param[in] lambda lambda (time + lambda * space), used to tradeoff
   * between time and space cost
   */
  CARMIExternalMap(const void *dataset, int record_number, int record_len,
                   double lambda) {
    KeyVectorType tmp;
    carmi_tree = carmi_impl(dataset, tmp, lambda, record_number, record_len);

    carmi_tree.Construction();
  }

  /**
   * @brief Construct a new CARMIExternalMap object with the historical insert
   * queries.
   *
   * The CARMI index structure can achieve a good performance under the
   * time/space setting based on the lambda parameter. The goal of the hybrid
   * construction algorithm is to minimize the total cost of historical access
   * and insert queries, which is: the average time cost of each query + lambda
   * times the space cost of the index tree.
   *
   * @param[in] dataset the pointer to the external dataset used to construct
   * the index and the historical find queries
   * @param[in] future_insert the array of keywords that may be inserted in
   * the future to reserve space for them, which is also the historical insert
   * queries
   * @param[in] record_number the number of the records
   * @param[in] record_len the length of a record (byte)
   * @param[in] lambda lambda (time + lambda * space), used to tradeoff
   * between time and space cost
   */
  CARMIExternalMap(const void *dataset, const KeyVectorType &future_insert,
                   int record_number, int record_len, double lambda) {
    carmi_tree =
        carmi_impl(dataset, future_insert, lambda, record_number, record_len);

    carmi_tree.Construction();
  }

 public:
  /// *** Fast Copy: Assign Operator and Copy Constructors

  /**
   * @brief Construct a new CARMIExternalMap object using the copy constructor
   *
   * @param[in] x the other carmi external map object
   */
  CARMIExternalMap(const CARMIExternalMap &x) : carmi_tree(x.carmi_tree) {}

  /**
   * @brief Construct a new CARMIExternalMap object using the copy constructor
   *
   * @param[in] x the other carmi external map object
   * @param[in] alloc The allocator object used to define the storage allocation
   * model.
   */
  CARMIExternalMap(const CARMIExternalMap &x, const Alloc &alloc)
      : carmi_tree(x.carmi_tree), allocator_(alloc) {}

  /**
   * @brief Assignment operator. All the key/data pairs are copied
   *
   * @param[in] other the given carmi map object
   * @return CARMIExternalMap& this object
   */
  inline CARMIExternalMap &operator=(const CARMIExternalMap &other) {
    if (this != &other) {
      carmi_tree = other.carmi_tree;
      allocator_ = other.allocator_;
      key_less_ = other.key_less_;
    }
    return *this;
  }

  class iterator;
  class const_iterator;
  class reverse_iterator;
  class const_reverse_iterator;

  /**
   * @brief the iterator of CARMIExternalMap items. The iterator points to a
   * specific position of a record in the carmi_tree and provides some basic
   * functions, such as: get the key value, get the data, get the next data
   * point and so on. Users can use this iterator to get the key/value of a data
   * point stored in the carmi_tree.
   */
  class iterator {
   public:
    /**
     * @brief Construct an empty new iterator object with the default values
     */
    inline iterator() : tree(NULL), currslot(0) {}

    /**
     * @brief Construct a new iterator object and set the pointer to the current
     * carmi index
     *
     * @param[in] t the pointer of the carmi tree
     */
    explicit inline iterator(CARMIExternalMap *t) : tree(t), currslot(0) {}

    /**
     * @brief Construct a new iterator object
     *
     * @param[in] t the pointer of the carmi tree
     * @param[in] slot the index of the records in the data block
     */
    inline iterator(CARMIExternalMap *t, int slot) : tree(t), currslot(slot) {}

    /**
     * @brief Construct a new iterator object from a reverse iterator
     *
     * @param[in] it the given reverse iterator
     */
    explicit inline iterator(const reverse_iterator &it)
        : tree(it.tree), currslot(it.currslot) {}

    /**
     * @brief get the key value of this iterator
     *
     * @return const KeyType& the key value
     * @retval DBL_MAX the record is invalid
     */
    inline const KeyType key() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (tree == NULL || currslot < 0 ||
          currslot >= tree->carmi_tree.currsize) {
        return DBL_MAX;
      }
      // Case 2: the current iterator is valid, return the key value in the data
      // block
      return *(static_cast<const char *>(tree->carmi_tree.external_data) +
               currslot * tree->carmi_tree.recordLength);
    }

    /**
     * @brief get the data value of this iterator
     *
     * @return const KeyVectorType& the data value
     * @retval KeyVectorType() the record is invalid
     */
    inline const KeyVectorType data() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (tree == NULL || currslot < 0 ||
          currslot >= tree->carmi_tree.currsize) {
        return KeyVectorType();
      }
      // Case 2: the current iterator is valid, return the key value in the data
      // block
      KeyVectorType res(tree->carmi_tree.recordLength / sizeof(KeyType) - 1);
      for (int i = 0; i < res.size(); i++) {
        res[i] = *(static_cast<const char *>(tree->carmi_tree.external_data) +
                   currslot * tree->carmi_tree.recordLength +
                   (i + 1) * sizeof(KeyType));
      }
      return res;
    }

    /**
     * @brief Return a reference of the record of this iterator
     *
     * @return KeyVectorType the reference of this record
     */
    inline DataType operator*() const {
      DataType res;
      res.first = key();
      res.second = data();
      return res;
    }

    /**
     * @brief Return the pointer to this record
     *
     * @return DataType* the pointer to this record
     */
    inline DataType *operator->() const {
      DataType *res;
      res->first = key();
      res->second = data();
      return res;
    }

    /**
     * @brief check if the given iterator x is equal to this iterator
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is equal to this iterator
     * @retval false the given iterator is unequal to this iterator
     */
    inline bool operator==(const iterator &x) const {
      return x.tree == tree && x.currslot == currslot;
    }

    /**
     * @brief check if the given iterator x is unequal to this iterator
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is unequal to this iterator
     * @retval false the given iterator is equal to this iterator
     */
    inline bool operator!=(const iterator &x) const {
      return x.tree != tree || x.currslot != currslot;
    }

    /**
     * @brief Prefix++ get the iterator of the next record
     *
     * @return iterator& the next iterator
     */
    inline iterator &operator++() {
      if (currslot + 1 >= tree->carmi_tree.currsize) {
        *this = this->tree->end();
        return *this;
      }
      currslot++;
      return *this;
    }

    /**
     * @brief Postfix++ get the iterator of the next record
     *
     * @return iterator& the next iterator
     */
    inline iterator operator++(int) {
      iterator tmp = *this;  // copy ourselves
      if (currslot + 1 >= tree->carmi_tree.currsize) {
        *this = this->tree->end();
        return tmp;
      }
      currslot++;
      return tmp;
    }

    /**
     * @brief Prefix-- get the iterator of the previous record
     *
     * @return iterator& the previous iterator
     */
    inline iterator &operator--() {
      if (currslot - 1 < 0) {
        *this = this->tree->end();
        return *this;
      }
      currslot--;
      return *this;
    }

    /**
     * @brief Postfix-- get the iterator of the previous record
     *
     * @return iterator& the previous iterator
     */
    inline iterator operator--(int) {
      iterator tmp = *this;  // copy ourselves
      if (currslot - 1 < 0) {
        *this = this->tree->end();
        return tmp;
      }
      currslot--;
      return tmp;
    }

   public:
    //*** Public Data Members of Iterator Objects

    /**
     * @brief the pointer of the carmi tree
     */
    CARMIExternalMap *tree;

    /**
     * @brief the index of the record in the external dataset
     */
    int currslot;
  };

  class const_iterator {
   public:
    /**
     * @brief Construct an empty new const iterator object with the default
     * values
     */
    inline const_iterator() {}

    /**
     * @brief Construct a new const iterator object and set the pointer to the
     * current carmi index
     *
     * @param[in] t the pointer of the carmi tree
     */
    explicit inline const_iterator(CARMIExternalMap *t)
        : tree(t), currslot(0) {}

    /**
     * @brief Construct a new const iterator object
     *
     * @param[in] t the pointer of the carmi tree
     * @param[in] slot the index of the records in the data block
     */
    inline const_iterator(CARMIExternalMap *t, int slot)
        : tree(t), currslot(slot) {}

    /**
     * @brief Construct a new const iterator object from an iterator
     *
     * @param[in] it the given iterator
     */
    explicit inline const_iterator(const iterator &it)
        : tree(it.tree), currslot(it.currslot) {}

    /**
     * @brief Construct a new const iterator object from a reverse iterator
     *
     * @param[in] it the given iterator
     */
    explicit inline const_iterator(const reverse_iterator &it)
        : tree(it.tree), currslot(it.currslot) {}

    /**
     * @brief Construct a new const iterator object from a const reverse
     * iterator
     *
     * @param[in] it the given iterator
     */
    explicit inline const_iterator(const const_reverse_iterator &it)
        : tree(it.tree), currslot(it.currslot) {}

    /**
     * @brief get the key value of this iterator
     *
     * @return const KeyType& the key value
     * @retval DBL_MAX the record is invalid
     */
    inline const KeyType &key() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (tree == NULL || currslot < 0 ||
          currslot >= tree->carmi_tree.currsize) {
        return DBL_MAX;
      }
      // Case 2: the current iterator is valid, return the key value in the data
      // block
      return *(static_cast<const char *>(tree->carmi_tree.external_data) +
               currslot * tree->carmi_tree.recordLength);
    }

    /**
     * @brief get the data value of this iterator
     *
     * @return const KeyVectorType& the data value
     * @retval KeyVectorType() the record is invalid
     */
    inline const KeyVectorType &data() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (tree == NULL || currslot < 0 ||
          currslot >= tree->carmi_tree.currsize) {
        return KeyVectorType();
      }
      // Case 2: the current iterator is valid, return the key value in the data
      // block
      KeyVectorType res(tree->carmi_tree.recordLength / sizeof(KeyType) - 1);
      for (int i = 0; i < res.size(); i++) {
        res[i] = *(static_cast<const char *>(tree->carmi_tree.external_data) +
                   currslot * tree->carmi_tree.recordLength +
                   (i + 1) * sizeof(KeyType));
      }
      return res;
    }

    /**
     * @brief Return a reference of the record of this iterator
     *
     * @return KeyVectorType& the reference of this record
     */
    inline DataType &operator*() const {
      DataType res;
      res.first = key();
      res.second = data();
      return res;
    }

    /**
     * @brief Return the pointer to this record
     *
     * @return DataType* the pointer to this record
     */
    inline DataType *operator->() const {
      DataType *res;
      res->first = key();
      res->second = data();
      return res;
    }

    /**
     * @brief check if the given iterator x is equal to this iterator
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is equal to this iterator
     * @retval false the given iterator is unequal to this iterator
     */
    inline bool operator==(const const_iterator &x) const {
      return x.tree == tree && x.currslot == currslot;
    }

    /**
     * @brief check if the given iterator x is unequal to this iterator
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is unequal to this iterator
     * @retval false the given iterator is equal to this iterator
     */
    inline bool operator!=(const const_iterator &x) const {
      return x.tree != tree || x.currslot != currslot;
    }

    /**
     * @brief Prefix++ get the iterator of the next record
     *
     * @return const_iterator& the next iterator
     */
    inline const_iterator &operator++() {
      if (currslot + 1 >= tree->carmi_tree.currsize) {
        *this = this->tree->cend();
        return *this;
      }
      currslot++;
      return *this;
    }

    /**
     * @brief Postfix++ get the iterator of the next record
     *
     * @return const_iterator& the next iterator
     */
    inline const_iterator operator++(int) {
      iterator tmp = *this;  // copy ourselves
      if (currslot + 1 >= tree->carmi_tree.currsize) {
        *this = this->tree->cend();
        return tmp;
      }
      currslot++;
      return tmp;
    }

    /**
     * @brief Prefix-- get the iterator of the previous record
     *
     * @return const_iterator& the previous iterator
     */
    inline const_iterator &operator--() {
      if (currslot - 1 < 0) {
        *this = this->tree->cend();
        return *this;
      }
      currslot--;
      return *this;
    }

    /**
     * @brief Postfix-- get the iterator of the previous record
     *
     * @return const_iterator& the previous iterator
     */
    inline const_iterator operator--(int) {
      iterator tmp = *this;  // copy ourselves
      if (currslot - 1 < 0) {
        *this = this->tree->cend();
        return tmp;
      }
      currslot--;
      return tmp;
    }

   public:
    //*** Public Data Members of Iterator Objects

    /**
     * @brief the pointer of the carmi tree
     */
    CARMIExternalMap *tree;

    /**
     * @brief the index of the record in the external dataset
     */
    int currslot;
  };

  class reverse_iterator {
   public:
    /**
     * @brief Construct an empty new reverse iterator object with the default
     * values
     */
    inline reverse_iterator() {}

    /**
     * @brief Construct a new reverse iterator object and set the pointer to the
     * current carmi index
     *
     * @param[in] t the pointer of the carmi tree
     */
    explicit inline reverse_iterator(CARMIExternalMap *t)
        : tree(t), currslot(0) {}

    /**
     * @brief Construct a new reverse iterator object
     *
     * @param[in] t the pointer of the carmi tree
     * @param[in] slot the index of the records in the data block
     */
    inline reverse_iterator(CARMIExternalMap *t, int slot)
        : tree(t), currslot(slot) {}

    /**
     * @brief Construct a new reverse iterator object from an iterator
     *
     * @param[in] it the given iterator
     */
    explicit inline reverse_iterator(const iterator &it)
        : tree(it.tree), currslot(it.currslot) {}

    /**
     * @brief Construct a new reverse iterator object from a reverse iterator
     *
     * @param[in] it the given iterator
     */
    explicit inline reverse_iterator(const reverse_iterator &it)
        : tree(it.tree), currslot(it.currslot) {}

    /**
     * @brief get the key value of this iterator
     *
     * @return const KeyType& the key value
     * @retval DBL_MAX the record is invalid
     */
    inline const KeyType &key() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (tree == NULL || currslot < 1 ||
          currslot > tree->carmi_tree.currsize) {
        return DBL_MAX;
      }
      // Case 2: the current iterator is valid, return the key value in the data
      // block
      return *(static_cast<const char *>(tree->carmi_tree.external_data) +
               (currslot - 1) * tree->carmi_tree.recordLength);
    }

    /**
     * @brief get the data value of this iterator
     *
     * @return const KeyVectorType& the data value
     * @retval KeyVectorType() the record is invalid
     */
    inline const KeyVectorType &data() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (tree == NULL || currslot < 1 ||
          currslot > tree->carmi_tree.currsize) {
        return DBL_MAX;
      }
      // Case 2: the current iterator is valid, return the key value in the data
      // block
      KeyVectorType res(tree->carmi_tree.recordLength / sizeof(KeyType) - 1);
      for (int i = 0; i < res.size(); i++) {
        res[i] = *(static_cast<const char *>(tree->carmi_tree.external_data) +
                   (currslot - 1) * tree->carmi_tree.recordLength +
                   (i + 1) * sizeof(KeyType));
      }
      return res;
    }

    /**
     * @brief Return a reference of the record of this iterator
     *
     * @return KeyVectorType& the reference of this record
     */
    inline DataType &operator*() const {
      DataType res;
      res.first = key();
      res.second = data();
      return res;
    }

    /**
     * @brief Return the pointer to this record
     *
     * @return DataType* the pointer to this record
     */
    inline DataType *operator->() const {
      DataType *res;
      res->first = key();
      res->second = data();
      return res;
    }

    /**
     * @brief check if the given iterator x is equal to this iterator
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is equal to this iterator
     * @retval false the given iterator is unequal to this iterator
     */
    inline bool operator==(const reverse_iterator &x) const {
      return x.tree == tree && x.currslot == currslot;
    }

    /**
     * @brief check if the given iterator x is unequal to this iterator
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is unequal to this iterator
     * @retval false the given iterator is equal to this iterator
     */
    inline bool operator!=(const reverse_iterator &x) const {
      return x.tree != tree || x.currslot != currslot;
    }

    /**
     * @brief Prefix++ get the iterator of the next record
     *
     * @return reverse_iterator& the next iterator
     */
    inline reverse_iterator &operator++() {
      if (currslot - 1 <= 0) {
        *this = this->tree->rend();
        return *this;
      }
      currslot--;
      return *this;
    }

    /**
     * @brief Postfix++ get the iterator of the next record
     *
     * @return reverse_iterator& the next iterator
     */
    inline reverse_iterator operator++(int) {
      iterator tmp = *this;  // copy ourselves
      if (currslot - 1 <= 0) {
        *this = this->tree->rend();
        return tmp;
      }
      currslot--;
      return tmp;
    }

    /**
     * @brief Prefix-- get the iterator of the previous record
     *
     * @return reverse_iterator& the previous iterator
     */
    inline reverse_iterator &operator--() {
      if (currslot + 1 > tree->carmi_tree.currsize) {
        *this = this->tree->rend();
        return *this;
      }
      currslot++;
      return *this;
    }

    /**
     * @brief Postfix-- get the iterator of the previous record
     *
     * @return reverse_iterator& the previous iterator
     */
    inline reverse_iterator operator--(int) {
      iterator tmp = *this;  // copy ourselves
      if (currslot + 1 >= tree->carmi_tree.currsize) {
        *this = this->tree->rend();
        return tmp;
      }
      currslot++;
      return tmp;
    }

   public:
    //*** Public Data Members of Iterator Objects

    /**
     * @brief the pointer of the carmi tree
     */
    CARMIExternalMap *tree;

    /**
     * @brief the index of the record in the external dataset
     */
    int currslot;
  };

  class const_reverse_iterator {
   public:
    /**
     * @brief Construct an empty new const reverse iterator object with the
     * default values
     */
    inline const_reverse_iterator() {}

    /**
     * @brief Construct a new const reverse iterator object and set the pointer
     * to the current carmi index
     *
     * @param[in] t the pointer of the carmi tree
     */
    explicit inline const_reverse_iterator(CARMIExternalMap *t)
        : tree(t), currslot(0) {}

    /**
     * @brief Construct a new reverse iterator object
     *
     * @param[in] t the pointer of the carmi tree
     * @param[in] slot the index of the records in the data block
     */
    inline const_reverse_iterator(CARMIExternalMap *t, int slot)
        : tree(t), currslot(slot) {}

    /**
     * @brief Construct a new reverse iterator object from an iterator
     *
     * @param[in] it the given iterator
     */
    explicit inline const_reverse_iterator(const iterator &it)
        : tree(it.tree), currslot(it.currslot) {}

    /**
     * @brief Construct a new reverse iterator object from a const iterator
     *
     * @param[in] it the given iterator
     */
    explicit inline const_reverse_iterator(const const_iterator &it)
        : tree(it.tree), currslot(it.currslot) {}

    /**
     * @brief Construct a new reverse iterator object from a reverse iterator
     *
     * @param[in] it the given iterator
     */
    explicit inline const_reverse_iterator(const reverse_iterator &it)
        : tree(it.tree), currslot(it.currslot) {}

    /**
     * @brief get the key value of this iterator
     *
     * @return const KeyType& the key value
     * @retval DBL_MAX the record is invalid
     */
    inline const KeyType &key() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (tree == NULL || currslot < 1 ||
          currslot > tree->carmi_tree.currsize) {
        return DBL_MAX;
      }
      // Case 2: the current iterator is valid, return the key value in the data
      // block
      return *(static_cast<const char *>(tree->carmi_tree.external_data) +
               (currslot - 1) * tree->carmi_tree.recordLength);
    }

    /**
     * @brief get the data value of this iterator
     *
     * @return const KeyVectorType& the data value
     * @retval KeyVectorType() the record is invalid
     */
    inline const KeyVectorType &data() const {
      // Case 1: the current iterator is invalid, return DBL_MAX directly
      if (tree == NULL || currslot < 1 ||
          currslot > tree->carmi_tree.currsize) {
        return DBL_MAX;
      }
      // Case 2: the current iterator is valid, return the key value in the data
      // block
      KeyVectorType res(tree->carmi_tree.recordLength / sizeof(KeyType) - 1);
      for (int i = 0; i < res.size(); i++) {
        res[i] = *(static_cast<const char *>(tree->carmi_tree.external_data) +
                   (currslot - 1) * tree->carmi_tree.recordLength +
                   (i + 1) * sizeof(KeyType));
      }
      return res;
    }

    /**
     * @brief Return a reference of the record of this iterator
     *
     * @return DataType& the reference of this record
     */
    inline DataType &operator*() const {
      DataType res;
      res.first = key();
      res.second = data();
      return res;
    }

    /**
     * @brief Return the pointer to this record
     *
     * @return DataType* the pointer to this record
     */
    inline DataType *operator->() const {
      DataType *res;
      res->first = key();
      res->second = data();
      return res;
    }

    /**
     * @brief check if the given iterator x is equal to this iterator
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is equal to this iterator
     * @retval false the given iterator is unequal to this iterator
     */
    inline bool operator==(const const_reverse_iterator &x) const {
      return x.tree == tree && x.currslot == currslot;
    }

    /**
     * @brief check if the given iterator x is unequal to this iterator
     *
     * @param[in] x the given iterator
     * @retval true the given iterator is unequal to this iterator
     * @retval false the given iterator is equal to this iterator
     */
    inline bool operator!=(const const_reverse_iterator &x) const {
      return x.tree != tree || x.currslot != currslot;
    }

    /**
     * @brief Prefix++ get the iterator of the next record
     *
     * @return const_reverse_iterator& the next iterator
     */
    inline const_reverse_iterator &operator++() {
      if (currslot - 1 <= 0) {
        *this = this->tree->crend();
        return *this;
      }
      currslot--;
      return *this;
    }

    /**
     * @brief Postfix++ get the iterator of the next record
     *
     * @return const_reverse_iterator& the next iterator
     */
    inline const_reverse_iterator operator++(int) {
      iterator tmp = *this;  // copy ourselves
      if (currslot - 1 <= 0) {
        *this = this->tree->crend();
        return tmp;
      }
      currslot--;
      return tmp;
    }

    /**
     * @brief Prefix-- get the iterator of the previous record
     *
     * @return const_reverse_iterator& the previous iterator
     */
    inline const_reverse_iterator &operator--() {
      if (currslot + 1 > tree->carmi_tree.currsize) {
        *this = this->tree->crend();
        return *this;
      }
      currslot++;
      return *this;
    }

    /**
     * @brief Postfix-- get the iterator of the previous record
     *
     * @return const_reverse_iterator& the previous iterator
     */
    inline const_reverse_iterator operator--(int) {
      iterator tmp = *this;  // copy ourselves
      if (currslot + 1 >= tree->carmi_tree.currsize) {
        *this = this->tree->crend();
        return tmp;
      }
      currslot++;
      return tmp;
    }

   public:
    //*** Public Data Members of Iterator Objects

    /**
     * @brief the pointer of the carmi tree
     */
    CARMIExternalMap *tree;

    /**
     * @brief the index of the record in the external dataset
     */
    int currslot;
  };

 public:
  // *** STL Iterator Construction Functions

  /**
   * @brief Returns an iterator referring to the first element in the carmi tree
   *
   * @return iterator
   */
  iterator begin() { return iterator(this); }

  /**
   * @brief get the end of the carmi tree
   *
   * @return iterator
   */
  iterator end() {
    iterator it(this);
    it.currslot = -1;
    return it;
  }

  /**
   * @brief Returns a const iterator referring to the first element in the carmi
   * tree
   *
   * @return const_iterator
   */
  const_iterator cbegin() const { return const_iterator(this); }

  /**
   * @brief get the end of the carmi tree
   *
   * @return const_iterator
   */
  const_iterator cend() const {
    const_iterator it(this);
    it.currslot = -1;
    return it;
  }

  /**
   * @brief Returns a reverse iterator referring to the first element in the
   * carmi tree
   *
   * @return reverse_iterator
   */
  reverse_iterator rbegin() {
    reverse_iterator it(this);
    it.currslot = carmi_tree.currsize;
    return it;
  }

  /**
   * @brief get the end of the carmi tree
   *
   * @return reverse_iterator
   */
  reverse_iterator rend() {
    reverse_iterator it(this);
    it.currslot = -1;
    return it;
  }

  /**
   * @brief Returns a reverse iterator referring to the first element in the
   * carmi tree
   *
   * @return const_reverse_iterator
   */
  const_reverse_iterator crbegin() const {
    const_reverse_iterator it(this);
    it.currslot = carmi_tree.currsize;
    return it;
  }

  /**
   * @brief get the end of the carmi tree
   *
   * @return const_reverse_iterator
   */
  const_reverse_iterator crend() const {
    const_reverse_iterator it(this);
    it.currslot = -1;
    return it;
  }

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
    int tmp = 0;
    carmi_tree.Find(key, tmp, &it.currslot);
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
    int tmp = 0;
    carmi_tree.Find(key, tmp, &it.currslot);
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
    int tmp = 0;
    carmi_tree.Find(key, &tmp, &it.currslot);
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
    int tmp = 0;
    carmi_tree.Find(key, &tmp, &it.currslot);
    while (it != cend() && it.key() == key) {
      it++;
    }
    return it;
  }

  /**
   * @brief find the corresponding iterator of the given key value
   *
   * @param[in] key the given key value
   * @return iterator the iterator of the data point
   */
  iterator find(const KeyType &key) {
    iterator it(this);
    int tmp = 0;
    carmi_tree.Find(key, &tmp, &it.currslot);
    if (it.key() == key) {
      return it;
    } else {
      return end();
    }
  }

  /**
   * @brief find the corresponding iterator of the given key value
   *
   * @param[in] key the given key value
   * @return const_iterator the iterator of the data point
   */
  const_iterator find(const KeyType &key) const {
    const_iterator it(this);
    int tmp = 0;
    carmi_tree.Find(key, &tmp, &it.currslot);
    if (it.key() == key) {
      return it;
    } else {
      return cend();
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
    const_iterator it = lower_bound(key);
    size_t num_equal = 0;
    while (it != end() && key_equal(it.key(), key)) {
      num_equal++;
      ++it;
    }
    return num_equal;
  }

 public:
  //*** Public Modify Functions

  /**
   * @brief insert the given key into carmi
   *
   * @param[in] key the given key value
   * @return std::pair<iterator, bool> a pair, with its member pair::first set
   * to an iterator pointing to either the newly inserted element in the map.
   * The pair::second element in the pair is set to true if a new element was
   * inserted.
   */
  std::pair<iterator, bool> insert(const KeyType &key) {
    iterator it(this);
    int tmp = 0;
    auto res = carmi_tree.Insert({key, 0}, &tmp, &it.currslot);
    return {it, res.second};
  }

  /**
   * @brief insert the given data point into carmi. This interface is the same
   * as the ordinary interface. This interface is only added to keep consistent
   * with the format of stl::map.
   *
   * @param[in] position useless here
   * @param[in] key the given key value
   * @return std::pair<iterator, bool> a pair, with its member pair::first set
   * to an iterator pointing to either the newly inserted element in the map.
   * The pair::second element in the pair is set to true if a new element was
   * inserted.
   */
  iterator insert(const_iterator position, const KeyType &key) {
    iterator it(this);
    int tmp = 0;
    auto res = carmi_tree.Insert({key, 0}, &tmp, &it.currslot);
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
  //*** Other Public Modify Functions

  /**
   * @brief Fast swapping of two carmi tree objects.
   *
   * @param[in] other the other carmi tree
   */
  void swap(CARMIExternalMap &other) {
    std::swap(carmi_tree, other.carmi_tree);
  }

  /**
   * @brief Removes all elements
   */
  void clear() { carmi_tree.clear(); }

 public:
  //*** Public Element Access Functions

  /**
   * @brief inserts a new element with that key and returns a reference to its
   * value
   *
   * @param[in] key the given key value
   * @return KeyVectorType& the value of it
   */
  KeyVectorType &operator[](const KeyType &key) {
    iterator it(this);
    int tmp = 0;
    auto res = carmi_tree.Insert({key, DBL_MAX}, &tmp, &it.currslot);
    it.currnode = res.first;
    return *(it.data());
  }

  /**
   * @brief Returns a reference to the mapped value of the element identified
   * with key
   *
   * @param[in] key the given key value
   * @return KeyVectorType& the value of it
   */
  KeyVectorType &at(const KeyType &key) {
    iterator it = find(key);
    if (it == end()) {
      throw std::out_of_range("CARMI::at: input does not match any key.");
    } else {
      return *(it.data());
    }
  }

  /**
   * @brief Returns a reference to the mapped value of the element identified
   * with key
   *
   * @param[in] key the given key value
   * @return KeyVectorType& the value of it
   */
  const KeyVectorType &at(const KeyType &key) const {
    const_iterator it = find(key);
    if (it == cend()) {
      throw std::out_of_range("CARMI::at: input does not match any key.");
    } else {
      return *(it.data());
    }
  }

 public:
  // *** Access Functions to the Item Count

  /**
   * @brief Return the number of data points in the carmi_tree
   *
   * @return size_type the number of data points
   */
  inline size_t size() const { return carmi_tree.currsize; }

  /**
   * @brief Returns true if there is at least one data point in the carmi tree
   *
   * @retval true there is at least one data point in the carmi tree
   * @retval false there is no data point in the carmi tree
   */
  inline bool empty() const { return (size() == size_t(0)); }

  /**
   * @brief Returns the largest possible size of the B+ Tree. This is just a
   * function required by the STL standard, the B+ Tree can hold more items.
   *
   * @return size_t
   */
  inline size_t max_size() const { return size_t(-1); }

  /**
   * @brief calculate the space of carmi tree
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

 public:
  // *** Key and Value Comparison Function Objects

  /**
   * @brief Get the allocator object
   *
   * @return Alloc
   */
  Alloc get_allocator() const { return allocator_; }

  /**
   * @brief Returns a copy of the comparison object used by the container to
   * compare keys.
   *
   * @return Compare
   */
  Compare key_comp() const { return key_less_; }

  /**
   * @brief A class that uses the internal comparison object to generate the
   * appropriate comparison functional class.
   */
  class value_compare {
   protected:
    Compare comp;
    explicit value_compare(Compare c) : comp(c) {}

   public:
    /**
     * @brief compare two elements to get whether the key of the first one goes
     * before the second.
     *
     * @param[in] x the first object
     * @param[in] y the second object
     * @retval true the key of the first argument is considered to go before
     * that of the second
     * @retval false the key of the first argument is considered to go after
     * that of the second
     */
    bool operator()(const DataType &x, const DataType &y) const {
      return comp(x.first, y.first);
    }
  };

  /**
   * @brief Returns a comparison object that can be used to compare two elements
   * to get whether the key of the first one goes before the second.
   *
   * @return value_compare The comparison object for element values.
   */
  value_compare value_comp() const { return value_compare(key_less_); }
};

#endif  // CARMI_EXTERNAL_MAP_H_
