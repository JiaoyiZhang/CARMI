/**
 * @file inlineFunction.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_FUNC_INLINEFUNCTION_H_
#define SRC_CARMI_FUNC_INLINEFUNCTION_H_

#include <float.h>

#include <algorithm>
#include <vector>

#include "../../params.h"
#include "../carmi.h"

// search a key-value through binary search
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

// search a key-value through binary search
// in the gapped array
// return the idx of the first element >= key
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

// search a key-value through binary search in
// the YCSB leaf node
inline int CARMI::ExternalBinarySearch(double key, int start, int end) const {
  while (start < end) {
    int mid = (start + end) / 2;
    if (entireData[mid].first < key)
      start = mid + 1;
    else
      end = mid;
  }
  return start;
}

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
#endif  // SRC_CARMI_FUNC_INLINEFUNCTION_H_
