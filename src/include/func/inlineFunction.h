/**
 * @file inlineFunction.h
 * @author Jiaoyi
 * @brief the inline functions for public functions
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_FUNC_INLINEFUNCTION_H_
#define SRC_INCLUDE_FUNC_INLINEFUNCTION_H_

#include <float.h>

#include <algorithm>
#include <vector>

#include "../../params.h"
#include "../carmi.h"
#include "../construct/minor_function.h"

/**
 * @brief search a key-value through binary search
 *
 * @param key
 * @param start
 * @param end
 * @return int the index of the key
 */
inline int CARMI::ArrayBinarySearch(double key, int start, int end) const {
  while (start < end) {
    int mid = (start + end) / 2;
    if (entireData[mid].first < key)
      start = mid + 1;
    else
      end = mid;
  }
  return start;
}

/**
 * @brief search a key-value through binary search in the gapped array
 *
 * @param key
 * @param start_idx
 * @param end_idx
 * @return int the idx of the first element >= key
 */
inline int CARMI::GABinarySearch(double key, int start_idx, int end_idx) const {
  while (end_idx - start_idx >= 2) {
    int mid = (start_idx + end_idx) >> 1;
    if (entireData[mid].first == DBL_MIN) {
      if (entireData[mid - 1].first >= key)
        end_idx = mid - 1;
      else
        start_idx = mid + 1;
    } else {
      if (entireData[mid].first >= key)
        end_idx = mid;
      else
        start_idx = mid + 1;
    }
  }
  if (entireData[start_idx].first >= key)
    return start_idx;
  else
    return end_idx;
}

/**
 * @brief search a key-value through binary search in the external leaf node
 *
 * @param key
 * @param start
 * @param end
 * @return int the idx of the first element >= key
 */
inline int CARMI::ExternalBinarySearch(double key, int start, int end) const {
  while (start < end) {
    int mid = (start + end) / 2;
    if (externalData[mid].first < key)
      start = mid + 1;
    else
      end = mid;
  }
  return start;
}

/**
 * @brief the main function of search a record in array
 *
 * @param key the key value
 * @param preIdx the predicted index of this node
 * @param error the error bound of this node
 * @param left the left index of this node in the entireData
 * @param size the size of this node
 * @return int the index of the record
 */
inline int CARMI::ArraySearch(double key, int preIdx, int error, int left,
                              int size) const {
  int start = std::max(0, preIdx - error) + left;
  int end = std::min(size - 1, preIdx + error) + left;
  start = std::min(start, end);
  int res;
  if (key <= entireData[start].first)
    res = ArrayBinarySearch(key, left, start);
  else if (key <= entireData[end].first)
    res = ArrayBinarySearch(key, start, end);
  else
    res = ArrayBinarySearch(key, end, left + size - 1);
  return res;
}

/**
 * @brief the main function of search a record in gapped array
 *
 * @param key the key value
 * @param preIdx the predicted index of this node
 * @param error the error bound of this node
 * @param left the left index of this node in the entireData
 * @param maxIndex the max index of this node
 * @return int the index of the record
 */
inline int CARMI::GASearch(double key, int preIdx, int error, int left,
                           int maxIndex) const {
  int start = std::max(0, preIdx - error) + left;
  int end = std::min(maxIndex, preIdx + error) + left;
  start = std::min(start, end);

  int res;
  if (entireData[start].first == DBL_MIN) start--;
  if (entireData[end].first == DBL_MIN) end--;
  if (key <= entireData[start].first)
    res = GABinarySearch(key, left, start);
  else if (key <= entireData[end].first)
    res = GABinarySearch(key, start, end);
  else
    res = GABinarySearch(key, end, left + maxIndex);
  return res;
}

/**
 * @brief the main function of search a record in external array
 *
 * @param key the key value
 * @param preIdx the predicted index of this node
 * @param error the error bound of this node
 * @param left the left index of this node in the entireData
 * @param size the size of this node
 * @return int the index of the record
 */
inline int CARMI::ExternalSearch(double key, int preIdx, int error, int left,
                                 int size) const {
  int start = std::max(0, preIdx - error) + left;
  int end = std::min(size - 1, preIdx + error) + left;
  start = std::min(start, end);
  int res;
  if (key <= externalData[start].first)
    res = ExternalBinarySearch(key, left, start);
  else if (key <= externalData[end].first)
    res = ExternalBinarySearch(key, start, end);
  else
    res = ExternalBinarySearch(key, end, left + size - 1);
  return res;
}

/**
 * @brief split the current leaf node into an inner node and several leaf nodes
 *
 * @tparam TYPE the type of the current leaf node
 * @param isExternal check whether the current node is the external array
 * @param left the left index of this node in the entireData
 * @param size the size of this node
 * @param previousIdx the index of the previous leaf node
 * @param idx the index of the current leaf node
 */
template <typename TYPE>
inline void CARMI::Split(bool isExternal, int left, int size, int previousIdx,
                         int idx) {
  int actualSize = 0;
  carmi_params::DataVectorType tmpDataset = ExtractData(left, size, entireData, &actualSize);

  // create a new inner node
  auto node = LRModel();
  int childNum = carmi_params::kInsertNewChildNumber;
  node.SetChildNumber(childNum);
  node.childLeft = AllocateChildMemory(childNum);
  Train(0, actualSize, tmpDataset, &node);
  entireChild[idx].lr = node;

  std::vector<IndexPair> perSize(childNum, emptyRange);
  IndexPair range(0, actualSize);
  NodePartition<LRModel>(node, range, tmpDataset, &perSize);

  int tmpLeft = left;
  for (int i = 0; i < childNum; i++) {
    TYPE tmpLeaf(kThreshold);
    Init(kMaxKeyNum, perSize[i].left, perSize[i].size, tmpDataset, &tmpLeaf);
    if (isExternal) {
      tmpLeaf.m_left = tmpLeft;
      tmpLeft += perSize[i].size;
    }
    entireChild[node.childLeft + i].array =
        *(reinterpret_cast<ArrayType*>(&tmpLeaf));
  }

  if (!isExternal) {
    if (previousIdx >= 0)
      entireChild[previousIdx].array.nextLeaf = node.childLeft;
    entireChild[node.childLeft].array.previousLeaf = previousIdx;
    int end = node.childLeft + childNum - 1;
    for (int i = node.childLeft + 1; i < end; i++) {
      entireChild[i].array.previousLeaf = i - 1;
      entireChild[i].array.nextLeaf = i + 1;
    }
    entireChild[end].array.previousLeaf = end - 1;
  }
}

#endif  // SRC_INCLUDE_FUNC_INLINEFUNCTION_H_
