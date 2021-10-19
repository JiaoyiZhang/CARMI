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
#ifndef SRC_INCLUDE_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
#define SRC_INCLUDE_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_

#include <math.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "../../construct/structures.h"
#include "../../memoryLayout/data_array.h"
#include "../../params.h"

/**
 * @brief the external array leaf node
 *
 * @tparam KeyType the type of the keyword
 */
template <typename KeyType>
class ExternalArray {
 public:
  // *** Constructed Types and Constructor

  // the pair of data points
  typedef std::pair<KeyType, KeyType> DataType;

  // the vector of data points, which is the type of dataset
  typedef std::vector<DataType> DataVectorType;

  ExternalArray() {
    flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + 0;
    error = 0;
    m_left = -1;
    slope = 0.0001;
    intercept = 0.666;
  }

 public:
  // *** Static Functions of the External Array Leaf Node

  /**
   * @brief extract data points (delete useless gaps and deleted data points)
   *
   * @param external_data the dataset pointer
   * @param left the left index of dataset managed by the leaf node
   * @param right the right index of dataset managed by the leaf node
   * @param recordLength the length of a record
   * @param actualSize the actual size of these data points
   * @return DataVectorType : pure data points
   */
  static DataVectorType ExtractDataset(const void *external_data, int left,
                                       int right, int recordLength,
                                       int *actualSize);

 public:
  // *** Basic Functions of External Array Leaf Objects

  /**
   * @brief initialize cf array node
   *
   * @param dataset the dataset
   * @param start_idx the start index of data points
   * @param size the size of data points
   * @param data the data array
   */
  void Init(const DataVectorType &dataset,
            const std::vector<int> &prefetchIndex, int start_idx, int size,
            DataArrayStructure<KeyType, KeyType> *data);

  /**
   * @brief train the external array node
   *
   * @param start_idx the start index of data points
   * @param size the size of data points
   * @param dataset the dataset
   */
  void Train(int start_idx, int size, const DataVectorType &dataset);

  int Find(const KeyType &key, int recordLength,
           const void *external_data) const;

  bool Insert(const DataType &datapoint, int *curr);

  /**
   * @brief predict the position of the given key value
   *
   * @param key the given key value
   * @return int: the predicted index in the leaf node
   */
  inline int Predict(double key) const {
    int size = (flagNumber & 0x00FFFFFF);
    int p = (slope * key + intercept) * size;
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
   * @param key the key value
   * @param preIdx the predicted index of this node
   * @param error the error bound of this node
   * @param left the left index of this node in the data
   * @param size the size of this node
   * @param recordLength the length of a record
   * @param external_data the external position of dataset
   * @return int: the index of the record
   */
  int Search(KeyType key, int preIdx, int error, int left, int size,
             int recordLength, const void *external_data) const;

  /**
   * @brief search a key-value through binary search in the external leaf node
   *
   * @param key the given key value
   * @param start the start index of the search bounary
   * @param end the end index of the search boundary
   * @param recordLength the length of a record
   * @param external_data the external position of dataset
   * @return int: the idx of the first element >= key
   */
  int BinarySearch(KeyType key, int start, int end, int recordLength,
                   const void *external_data) const;

  /**
   * @brief find the optimal error value from 0 to size
   *
   * @param start_idx the start index of the data points
   * @param size the size of the data points
   * @param dataset the dataset
   */
  void FindOptError(int start_idx, int size, const DataVectorType &dataset);

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

  // The slope parameter of the linear regression model. (4 bytes)
  float slope;

  // The intercept parameter of the linear regression model. (4 bytes)
  float intercept;

  // Placeholder to make sure that the size of this node is 64 bytes. (44 bytes)
  float placeholder[11];
};

template <typename KeyType>
typename ExternalArray<KeyType>::DataVectorType
ExternalArray<KeyType>::ExtractDataset(const void *external_data, int left,
                                       int right, int recordLength,
                                       int *actualSize) {
  *actualSize = 0;
  int size = right - left;
  int maxBlockNum = carmi_params::kMaxLeafNodeSize / sizeof(DataType);
  DataVectorType currdata(size, {DBL_MAX, DBL_MAX});
  for (int i = left; i < right; i++) {
    KeyType tmpKey = *reinterpret_cast<const KeyType *>(
        static_cast<const char *>(external_data) + i * recordLength);
    if (tmpKey != DBL_MAX) {
      currdata[(*actualSize)++] = {tmpKey, tmpKey};
    }
  }
  return currdata;
}

template <typename KeyType>
inline void ExternalArray<KeyType>::Init(
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    int start_idx, int size, DataArrayStructure<KeyType, KeyType> *data) {
  m_left = start_idx;
  if (size == 0) return;

  Train(start_idx, size, dataset);
}

template <typename KeyType>
inline void ExternalArray<KeyType>::Train(int start_idx, int size,
                                          const DataVectorType &dataset) {
  DataVectorType currdata(size, {DBL_MAX, DBL_MAX});
  int end = start_idx + size;
  for (int i = start_idx, j = 0; i < end; i++, j++) {
    currdata[j].first = dataset[i].first;
    currdata[j].second = static_cast<double>(j) / size;
  }
  if (size == 0) return;

  if ((flagNumber & 0x00FFFFFF) != size) {
    flagNumber = (EXTERNAL_ARRAY_LEAF_NODE << 24) + size;
  }
  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = 0; i < size; i++) {
    t1 += dataset[i].first * dataset[i].first;
    t2 += dataset[i].first;
    t3 += dataset[i].first * dataset[i].second;
    t4 += dataset[i].second;
  }
  if (t1 * size - t2 * t2) {
    slope = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    intercept = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
  } else {
    slope = 0;
    intercept = 0;
  }
  FindOptError(0, size, currdata);
}

template <typename KeyType>
inline int ExternalArray<KeyType>::Find(const KeyType &key, int recordLength,
                                        const void *external_data) const {
  // Use the lr model to predict the position of the data point.
  int preIdx = Predict(key);
  // Get the size of data points in this leaf node.
  int size = flagNumber & 0x00FFFFFF;

  // Case 1: the predicted position is right.
  // Find the data point successfully and return the predicted index directly
  if (*reinterpret_cast<const KeyType *>(
          static_cast<const char *>(external_data) +
          (m_left + preIdx) * recordLength) == key) {
    return preIdx;
  }

  // Case 2: the predicted position is inaccurate.
  // Use binary search to find the accurate position.
  preIdx =
      Search(key, preIdx, error, m_left, size, recordLength, external_data);

  // Case 2.1: find the data point unsuccessfully, return 0.
  if (preIdx >= m_left + size || *reinterpret_cast<const KeyType *>(
                                     static_cast<const char *>(external_data) +
                                     preIdx * recordLength) != key) {
    return 0;
  }

  // Case 2.2: find the data poin successfully, return its index in this node.
  return preIdx - m_left;
}

template <typename KeyType>
bool ExternalArray<KeyType>::Insert(const DataType &datapoint, int *curr) {
  // Get the size of data points in this leaf node.
  int size = flagNumber & 0x00FFFFFF;
  // Case 1: insert fails.
  // If there is no empty position for the newly inserted data point,
  // return false directly.
  if (size >= carmi_params::kMaxLeafNodeSizeExternal) {
    return false;
  }

  if (size == 0) {
    m_left = *curr;
    DataVectorType trainData(1, datapoint);
    Train(0, 1, trainData);
  }

  flagNumber++;
  (*curr)++;
  return true;
}

template <typename KeyType>
inline int ExternalArray<KeyType>::Search(KeyType key, int preIdx, int error,
                                          int left, int size, int recordLength,
                                          const void *external_data) const {
  int start = std::max(0, preIdx - error) + left;
  int end = std::min(std::max(0, size - 1), preIdx + error) + left;
  start = std::min(start, end);
  int res;
  if (key <=
      *reinterpret_cast<const KeyType *>(
          static_cast<const char *>(external_data) + start * recordLength))
    res = BinarySearch(key, left, start, recordLength, external_data);
  else if (key <=
           *reinterpret_cast<const KeyType *>(
               static_cast<const char *>(external_data) + end * recordLength))
    res = BinarySearch(key, start, end, recordLength, external_data);
  else
    res = BinarySearch(key, end, left + size - 1, recordLength, external_data);
  return res;
}

template <typename KeyType>
inline int ExternalArray<KeyType>::BinarySearch(
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

template <typename KeyType>
void ExternalArray<KeyType>::FindOptError(int start_idx, int size,
                                          const DataVectorType &dataset) {
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
  int minRes = size * log2(size);
  int res;
  int cntBetween = 0;
  for (int e = 0; e <= size; e++) {
    if (error_count[e] == 0) {
      continue;
    }
    cntBetween += error_count[e];
    if (e != 0)
      res = cntBetween * log2(e) + (size - cntBetween) * log2(size);
    else
      res = (size - cntBetween) * log2(size);
    if (res < minRes) {
      minRes = res;
      error = e;
    }
  }
}

#endif  // SRC_INCLUDE_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
