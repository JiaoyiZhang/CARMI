/**
 * @file external_array_type.h
 * @author Jiaoyi
 * @brief external array leaf node
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
#define NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_

#include <math.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../../construct/structures.h"
#include "../../memoryLayout/data_array.h"
#include "../../params.h"

/**
 * @brief the external array leaf node
 *
 * @tparam KeyType Type of keys.
 * @tparam ValueType Type of value.
 * @tparam Compare A binary predicate that takes two element keys as arguments
 * and returns a bool.
 */
template <typename KeyType, typename ValueType,
          typename Compare = std::less<KeyType>>
class ExternalArray {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief the pair of data points
   */
  typedef std::pair<KeyType, ValueType> DataType;

  /**
   * @brief the vector of data points, which is the type of dataset
   *
   */
  typedef std::vector<DataType> DataVectorType;

  ExternalArray() {
    flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + 0;
    error = 0;
    m_left = -1;
    slope = 0.0001;
    intercept = 0.666;
    minValue = 0;
    for (int i = 0; i < 11; i++) {
      placeholder[i] = 0;
    }
  }

 public:
  // *** Static Functions of the External Array Leaf Node

  /**
   * @brief extract data points (delete useless gaps and deleted data points)
   *
   * @param[in] external_data the dataset pointer
   * @param[in] left the left index of dataset managed by the leaf node
   * @param[in] right the right index of dataset managed by the leaf node
   * @param[in] recordLength the length of a record
   * @return DataVectorType : pure data points
   */
  static DataVectorType ExtractDataset(const void *external_data, int left,
                                       int right, int recordLength);

 public:
  // *** Basic Functions of External Array Leaf Objects

  /**
   * @brief initialize the external array node
   *
   * @param[in] dataset the dataset
   * @param[in] prefetchIndex this parameter is to be consistent with the cf
   * leaf node. It is useless here.
   * @param[in] start_idx the starting index of data points in the dataset, the
   * size of the sub-dataset is equal to the size of the prefetchIndex vector
   * @param[in] data this parameter is to be consistent with the cf leaf
   * node. It is useless here.
   */
  void Init(
      const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
      int start_idx,
      DataArrayStructure<KeyType, KeyType,
                         std::allocator<LeafSlots<KeyType, KeyType>>> *data);

  /**
   * @brief train the external array node
   *
   * @param[in] dataset the dataset
   * @param[in] start_idx the start index of data points in the dataset
   * @param[in] size the size of data points
   */
  void Train(const DataVectorType &dataset, int start_idx, int size);

  /**
   * @brief Find the data point of the given key value
   *
   * @param[in] key the given key value
   * @param[in] recordLength the length of the record
   * @param[in] external_data the external data
   * @return int the position of the record in the external dataset
   */
  int Find(const KeyType &key, int recordLength,
           const void *external_data) const;

  /**
   * @brief Insert the key value
   *
   * @param[in] datapoint the inserted data point
   * @param[inout] curr the current index of the insert
   * @retval true the operation is successful
   * @retval false the operation fails
   */
  bool Insert(const DataType &datapoint, int *curr);

  /**
   * @brief use the lr model to predict the position of the given key value
   *
   * @param[in] key the given key value
   * @return int: the predicted index in the leaf node
   */
  inline int Predict(KeyType key) const {
    int size = (flagNumber & 0x00FFFFFF);
    int p = (slope * static_cast<double>(key - minValue) + intercept) * size;
    if (p < 0)
      p = 0;
    else if (p >= size && size != 0)
      p = size - 1;
    return p;
  }

 private:
  //*** Private Sub-Functions of Find Function

  /**
   * @brief the main function of search a record in external array
   *
   * @param[in] key the key value
   * @param[in] preIdx the predicted index of this node
   * @param[in] recordLength the length of a record
   * @param[in] external_data the external position of dataset
   * @return int: the index of the record
   */
  int Search(KeyType key, int preIdx, int recordLength,
             const void *external_data) const;

  /**
   * @brief search a key-value through binary search in the external leaf
   node
   *
   * @param[in] key the given key value
   * @param[in] start the start index of the search bounary
   * @param[in] end the end index of the search boundary
   * @param[in] recordLength the length of a record
   * @param[in] external_data the external position of dataset
   * @return int: the idx of the first element >= key
   */
  int BinarySearch(KeyType key, int start, int end, int recordLength,
                   const void *external_data) const;

 private:
  //*** Private Sub-Functions of Train Function

  /**
   * @brief find the optimal error value from 0 to size
   *
   * @param[in] start_idx the start index of the data points
   * @param[in] size the size of the data points
   * @param[in] dataset the dataset
   */
  void FindOptError(int start_idx, int size, const DataVectorType &dataset);

  /**
   * @brief The bytes of placeholder.
   */
  static constexpr int kPlaceHolderLen = 40 - sizeof(KeyType);

 public:
  //*** Public Data Members of External Array Leaf Node Objects

  /**
   * @brief  A combined integer, composed of the flag of external array leaf
   * node and the number of data blocks managed by it. (4 bytes)
   */
  int flagNumber;

  /**
   * @brief The first index of data blocks managed by this object in the data
   * array. (4 bytes)
   */
  int m_left;

  /**
   * @brief The error parameter, used to limit the range of the first binary
   * search operation when the prediction is inaccurate. (4 bytes)
   * loc(x) is in [preIdx(x) - error / 2, preIdx + error / 2]
   */
  int error;

  /**
   * @brief The slope parameter of the linear regression model. (4 bytes)
   */
  float slope;

  /**
   * @brief The intercept parameter of the linear regression model. (4 bytes)
   */
  float intercept;

  /**
   * @brief The minimum value.
   */
  KeyType minValue;

  /**
   * @brief Placeholder, used to make sure that the size of this node is 64
   * bytes. (kPlaceHolderLen bytes)
   */
  char placeholder[kPlaceHolderLen];
};

template <typename KeyType, typename DataType, typename Compare>
typename ExternalArray<KeyType, DataType, Compare>::DataVectorType
ExternalArray<KeyType, DataType, Compare>::ExtractDataset(
    const void *external_data, int left, int right, int recordLength) {
  if (left < 0 || left > right) {
    throw std::out_of_range(
        "ExternalArray::DataVectorType: the range is invalid.");
  }
  DataVectorType currdata;
  for (int i = left; i < right; i++) {
    KeyType tmpKey = *reinterpret_cast<const KeyType *>(
        static_cast<const char *>(external_data) + i * recordLength);
    if (tmpKey != DBL_MAX) {
      currdata.push_back({tmpKey, tmpKey});
    }
  }
  return currdata;
}

template <typename KeyType, typename DataType, typename Compare>
inline void ExternalArray<KeyType, DataType, Compare>::Init(
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    int start_idx,
    DataArrayStructure<KeyType, KeyType,
                       std::allocator<LeafSlots<KeyType, KeyType>>> *data) {
  int size = prefetchIndex.size();
  if (size == 0) return;
  if (start_idx < 0) {
    throw std::out_of_range("ExternalArray::Init: the range is invalid.");
  }
  m_left = start_idx;

  Train(dataset, start_idx, size);
}

template <typename KeyType, typename DataType, typename Compare>
inline void ExternalArray<KeyType, DataType, Compare>::Train(
    const DataVectorType &dataset, int start_idx, int size) {
  if (size == 0) return;
  if (start_idx < 0 || size < 0 || start_idx + size > dataset.size()) {
    throw std::out_of_range("ExternalArray::Train: the range is invalid.");
  }
  typedef std::vector<std::pair<KeyType, double>> TrainType;
  TrainType currdata(size, {static_cast<KeyType>(DBL_MAX), DBL_MAX});
  int end = start_idx + size;
  minValue = dataset[start_idx].first;
  for (int i = start_idx, j = 0; i < end; i++, j++) {
    currdata[j].first = dataset[i].first - minValue;
    currdata[j].second = static_cast<double>(j) / size;
  }

  if ((flagNumber & 0x00FFFFFF) != size) {
    flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + size;
  }
  // train the lr model
  long double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = 0; i < size; i++) {
    t1 += static_cast<long double>(currdata[i].first) *
          static_cast<long double>(currdata[i].first);
    t2 += static_cast<long double>(currdata[i].first);
    t3 += static_cast<long double>(currdata[i].first) *
          static_cast<long double>(currdata[i].second);
    t4 += static_cast<long double>(currdata[i].second);
  }
  // update the parameters
  if (t1 * size - t2 * t2) {
    slope = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    intercept = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
  } else {
    slope = 0;
    intercept = 0;
  }
  // update the error parameter
  FindOptError(0, size, dataset);
}

template <typename KeyType, typename DataType, typename Compare>
inline int ExternalArray<KeyType, DataType, Compare>::Find(
    const KeyType &key, int recordLength, const void *external_data) const {
  // Use the lr model to predict the position of the data point.
  int preIdx = Predict(key);

  // Case 1: the predicted position is right.
  // Find the data point successfully and return the predicted index directly
  if (*reinterpret_cast<const KeyType *>(
          static_cast<const char *>(external_data) +
          (m_left + preIdx) * recordLength) == key) {
    return m_left + preIdx;
  }

  // Case 2: the predicted position is inaccurate.
  // Use binary search to find the accurate position.
  preIdx = Search(key, preIdx, recordLength, external_data);

  // Case 2.1: find the data point unsuccessfully, return 0.
  if (preIdx >= m_left + (flagNumber & 0x00FFFFFF)) {
    return -1;
  }

  // Case 2.2: find the data poin successfully, return its index in the external
  // dataset.
  return preIdx;
}

template <typename KeyType, typename DataType, typename Compare>
bool ExternalArray<KeyType, DataType, Compare>::Insert(
    const DataType &datapoint, int *curr) {
  // Get the size of data points in this leaf node.
  int size = flagNumber & 0x00FFFFFF;
  // Case 1: insert fails.
  // If there is no empty position for the newly inserted data point,
  // return false directly.
  if (size >= carmi_params::kMaxLeafNodeSizeExternal) {
    return false;
  }

  // Case 2: this node is empty
  // use this data point to train the lr model
  if (size == 0) {
    m_left = *curr;
    DataVectorType trainData(1, datapoint);
    Train(trainData, 0, 1);
  }
  // Case 3: update the parameters and return the current index of the insert
  flagNumber++;
  *curr = (*curr) + 1;
  return true;
}

template <typename KeyType, typename DataType, typename Compare>
inline int ExternalArray<KeyType, DataType, Compare>::Search(
    KeyType key, int preIdx, int recordLength,
    const void *external_data) const {
  int start = std::max(0, preIdx - error) + m_left;
  int size = flagNumber & 0x00FFFFFF;
  int end = std::min(std::max(0, size - 1), preIdx + error) + m_left;
  start = std::min(start, end);
  int res;
  // search between [m_left, start]
  if (key <=
      *reinterpret_cast<const KeyType *>(
          static_cast<const char *>(external_data) + start * recordLength))
    res = BinarySearch(key, m_left, start, recordLength, external_data);
  // search between [start, end]
  else if (key <=
           *reinterpret_cast<const KeyType *>(
               static_cast<const char *>(external_data) + end * recordLength))
    res = BinarySearch(key, start, end, recordLength, external_data);
  // search between [end, m_left + size - 1]
  else
    res =
        BinarySearch(key, end, m_left + size - 1, recordLength, external_data);
  return res;
}

template <typename KeyType, typename DataType, typename Compare>
inline int ExternalArray<KeyType, DataType, Compare>::BinarySearch(
    KeyType key, int start, int end, int recordLength,
    const void *external_data) const {
  while (start < end) {
    int mid = (start + end) / 2;
    if (*reinterpret_cast<const KeyType *>(
            static_cast<const char *>(external_data) + mid * recordLength) <
        key)
      start = mid + 1;
    else
      end = mid;
  }
  return start;
}

template <typename KeyType, typename DataType, typename Compare>
void ExternalArray<KeyType, DataType, Compare>::FindOptError(
    int start_idx, int size, const DataVectorType &dataset) {
  std::vector<int> error_count(size + 1, 0);

  // record each difference
  int p, d;
  int end = start_idx + size;
  for (int i = start_idx; i < end; i++) {
    p = Predict(dataset[i].first);
    d = abs(i - start_idx - p);

    error_count[d]++;
  }

  // find the optimal value of error
  int minRes = static_cast<double>(size) * log2(size);
  int res;
  int cntBetween = 0;
  for (int e = 0; e <= size; e++) {
    if (error_count[e] == 0) {
      continue;
    }
    cntBetween += error_count[e];
    if (e != 0)
      res = static_cast<double>(cntBetween) * log2(e) +
            static_cast<double>(size - cntBetween) * log2(size);
    else
      res = static_cast<double>(size - cntBetween) * log2(size);
    if (res < minRes) {
      minRes = res;
      error = e;
    }
  }
}

#endif  // NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
