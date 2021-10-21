/**
 * @file carmi_external.h
 * @author Jiaoyi
 * @brief the external CARMI
 * @version 3.0
 * @date 2021-04-01
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CARMI_EXTERNAL_H_
#define CARMI_EXTERNAL_H_

#include <utility>
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
 * @brief The specific external version of the CARMI class template.
 *
 * The implementation of the external CARMI. This class is designed for primary
 * index structures, where the original data points are already sorted according
 * to the key value and stored in an external location. In such a case, we only
 * need to store pointers to external locations in the leaf node.
 *
 * This class provides users with basic operations such as find, insert, update,
 * and delete, as well as some basic operations for the iterator of the CARMI
 * index. Through the functions of CARMICommon objects, users can construct
 * indexes with good performance for a given dataset without the need of too
 * much space. In addition, during initialization, users can set different
 * lambda values to control the different proportions of time and space costs,
 * so as to achieve a balance between the time and space cost under the
 * corresponding parameters.
 *
 * @tparam KeyType the type of the given key value
 */
template <typename KeyType>
class CARMIExternal {
 public:
  // *** Constructed Types

  /**
   * @brief the type of implementation of CARMI
   */
  typedef CARMI<KeyType, KeyType> carmi_impl;

  /**
   * @brief the type of the data point
   */
  typedef typename CARMI<KeyType, KeyType>::DataType DataType;

  /**
   * @brief the type of the dataset
   */
  typedef typename CARMI<KeyType, KeyType>::DataVectorType DataVectorType;

 private:
  /**
   * @brief The contained implementation object of CARMI
   */
  carmi_impl carmi_tree;

 public:
  /**
   * @brief Construct a new CARMIExternal object
   *
   * @param dataset[in] the dataset used to construct the index
   * @param future_insert[in] the array of keywords that may be inserted in the
   * future to reserve space for them
   * @param record_number[in] the number of the records
   * @param record_len[in] the length of a record (byte)
   * @param lambda[in] lambda (time + lambda * space), used to tradeoff between
   * time and space cost
   */
  CARMIExternal(const void *dataset, const std::vector<KeyType> future_insert,
                int record_number, int record_len, double lambda) {
    carmi_tree =
        carmi_impl(dataset, future_insert, lambda, record_number, record_len);

    carmi_tree.Construction();
  }

 public:
  // *** Basic Functions of CARMI External Objects

  /**
   * @brief find the corresponding iterator of the given key
   *
   * @param key[in] the given key value
   * @return void *: the pointer of the data point
   */
  const void *Find(const KeyType &key) {
    int idx = 0;
    int currblock = 0;
    auto currnode = carmi_tree.Find(key, &currblock, &idx);

    return static_cast<const void *>(
        static_cast<const char *>(carmi_tree.external_data) +
        (currnode->externalArray.m_left + idx) * carmi_tree.recordLength);
  }

  /**
   * @brief insert the given key into carmi
   *
   * @param key[in] the given key value
   */
  void Insert(const KeyType &key) { carmi_tree.Insert({key, key}); }

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
};

#endif  // CARMI_EXTERNAL_H_
