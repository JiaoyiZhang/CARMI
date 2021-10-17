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
#ifndef SRC_INCLUDE_CARMI_EXTERNAL_H_
#define SRC_INCLUDE_CARMI_EXTERNAL_H_

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
 * @brief the external CARMI
 *
 * @tparam KeyType the type of the given key
 */
template <typename KeyType>
class CARMIExternal {
 public:
  typedef CARMI<KeyType, KeyType>
      carmi_impl;  ///< the type of implementation of CARMI
  typedef typename CARMI<KeyType, KeyType>::DataType
      DataType;  ///< the type of the data point
  typedef typename CARMI<KeyType, KeyType>::DataVectorType
      DataVectorType;  ///< the type of the dataset

 private:
  carmi_impl carmi_tree;  ///< the implementation of CARMI

 public:
  /**
   * @brief Construct a new CARMIExternal object
   *
   * @param dataset the dataset used to construct the index
   * @param future_insert the array of keywords that may be inserted in the
   * future to reserve space for them
   * @param record_number the number of the records
   * @param record_len the length of a record (byte)
   * @param lambda  lambda (time + lambda * space)
   */
  CARMIExternal(const void *dataset, const std::vector<KeyType> future_insert,
                int record_number, int record_len, double lambda);

  // main functions
 public:
  /**
   * @brief find the corresponding iterator of the given key
   *
   * @param key the given key value
   * @return void *: the pointer
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
   * @param key the given key value
   */
  void Insert(const KeyType &key) { carmi_tree.Insert({key, key}); }

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
   * @param dataSize the size of the dataset
   * @param idx 0
   * @param levelVec the level of carmi
   * @param nodeVec the node type vector
   */
  void PrintStructure(int level, int dataSize, int idx,
                      std::vector<int> *levelVec, std::vector<int> *nodeVec) {
    carmi_tree.PrintStructure(level, PLR_ROOT_NODE, dataSize, idx, levelVec,
                              nodeVec);
  }
};

template <typename KeyType>
CARMIExternal<KeyType>::CARMIExternal(const void *dataset,
                                      const std::vector<KeyType> future_insert,
                                      int record_number, int record_len,
                                      double lambda) {
  DataVectorType initDataset(record_number, DataType());
  DataVectorType insertQuery(future_insert.size(), DataType());
  std::vector<int> insertQueryIndex(future_insert.size());
  DataVectorType findQuery = initDataset;

  for (int i = 0; i < record_number; i++) {
    initDataset[i] = {*reinterpret_cast<const KeyType *>(
                          static_cast<const char *>(dataset) + i * record_len),
                      1};
  }

  for (int i = 0; i < static_cast<int>(future_insert.size()); i++) {
    insertQuery[i] = {initDataset[i].first, 1};
    insertQueryIndex[i] = record_number + i;
  }

  carmi_tree = carmi_impl(dataset, initDataset, findQuery, insertQuery,
                          insertQueryIndex, lambda, record_number, record_len);

  carmi_tree.Construction();
}
#endif  // SRC_INCLUDE_CARMI_EXTERNAL_H_
