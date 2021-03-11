/**
 * @file array_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_LEAFNODE_ARRAY_TYPE_H_
#define SRC_CARMI_NODES_LEAFNODE_ARRAY_TYPE_H_

#include <float.h>

#include <utility>
#include <vector>

#include "../../../params.h"
#include "../../carmi.h"
#include "./array.h"

inline int ArrayType::Predict(double key) const {
  // return the predicted idx in the leaf node
  int size = (flagNumber & 0x00FFFFFF);
  int p = (theta1 * key + theta2) * size;
  if (p < 0)
    p = 0;
  else if (p >= size)
    p = size - 1;
  return p;
}

inline void CARMI::initArray(int cap, const int left, const int size,
                             const DataVectorType &dataset, ArrayType *arr) {
  if (arr->m_left != -1) {
    releaseMemory(arr->m_left, arr->m_capacity);
  }
  arr->m_capacity = cap;
  arr->flagNumber += size;
  while (size > arr->m_capacity) {
    arr->m_capacity *= kExpansionScale;
  }

  if (arr->m_capacity > 4096) {
    arr->m_capacity = 4096;
  }

  arr->m_left = allocateMemory(arr->m_capacity);
  if (size == 0) return;

  if (size > 4096)
    std::cout << "init Array setDataset WRONG! datasetSize > 4096, size is:"
              << size << std::endl;

  int end = left + size;
  for (int i = arr->m_left, j = left; j < end; i++, j++)
    entireData[i] = dataset[j];

  Train(arr);
  UpdateError(arr);
}

inline int CARMI::UpdateError(ArrayType *arr) {
  // find: max|pi-yi|
  int maxError = 0, p, d;
  int size = arr->flagNumber & 0x00FFFFFF;
  int end = arr->m_left + size;
  for (int i = arr->m_left; i < end; i++) {
    p = arr->Predict(entireData[i].first) + arr->m_left;
    d = abs(i - p);
    if (d > maxError) maxError = d;
  }

  // find the optimal value of error
  int minRes = size * log(size) / log(2);
  int res;
  int cntBetween, cntOut;
  for (int e = 0; e <= maxError; e++) {
    cntBetween = 0;
    cntOut = 0;
    for (int i = arr->m_left; i < end; i++) {
      p = arr->Predict(entireData[i].first) + arr->m_left;
      d = abs(i - p);
      if (d <= e)
        cntBetween++;
      else
        cntOut++;
    }
    if (e != 0)
      res = cntBetween * log(e) / log(2) + cntOut * log(size) / log(2);
    else
      res = cntOut * log(size) / log(2);
    if (res < minRes) {
      minRes = res;
      arr->error = e;
    }
  }
  return arr->error;
}

inline int CARMI::UpdateError(ArrayType *arr, const int start_idx,
                              const int size) {
  // find: max|pi-yi|
  int maxError = 0, p, d;
  for (int i = start_idx; i < start_idx + size; i++) {
    p = arr->Predict(initDataset[i].first);
    d = abs(i - start_idx - p);
    if (d > maxError) maxError = d;
  }

  // find the optimal value of error
  int minRes = size * log(size) / log(2);
  int res;
  int cntBetween, cntOut;
  for (int e = 0; e <= maxError; e++) {
    cntBetween = 0;
    cntOut = 0;
    for (int i = start_idx; i < start_idx + size; i++) {
      p = arr->Predict(initDataset[i].first);
      d = abs(i - start_idx - p);
      if (d <= e)
        cntBetween++;
      else
        cntOut++;
    }
    if (e != 0)
      res = cntBetween * log(e) / log(2) + cntOut * log(size) / log(2);
    else
      res = cntOut * log(size) / log(2);
    if (res < minRes) {
      minRes = res;
      arr->error = e;
    }
  }
  return arr->error;
}

inline void CARMI::Train(ArrayType *arr) {
  int actualSize = 0;
  std::vector<double> index;
  int size = arr->flagNumber & 0x00FFFFFF;
  int end = arr->m_left + size;
  for (int i = arr->m_left; i < end; i++) {
    if (entireData[i].first != DBL_MIN) actualSize++;
    index.push_back(static_cast<double>(i - arr->m_left) / size);
  }
  if (actualSize == 0) return;

  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = arr->m_left; i < end; i++) {
    if (entireData[i].first != DBL_MIN) {
      t1 += entireData[i].first * entireData[i].first;
      t2 += entireData[i].first;
      t3 += entireData[i].first * index[i - arr->m_left];
      t4 += index[i - arr->m_left];
    }
  }
  arr->theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
  arr->theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);
}

inline void CARMI::Train(ArrayType *arr, const int start_idx, const int size) {
  if ((arr->flagNumber & 0x00FFFFFF) != size) arr->flagNumber += size;
  std::vector<double> index;
  int end = start_idx + size;
  for (int i = start_idx; i < end; i++)
    index.push_back(static_cast<double>(i - start_idx) / size);

  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = start_idx; i < end; i++) {
    t1 += initDataset[i].first * initDataset[i].first;
    t2 += initDataset[i].first;
    t3 += initDataset[i].first * index[i - start_idx];
    t4 += index[i - start_idx];
  }
  arr->theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
  arr->theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
}

#endif  // SRC_CARMI_NODES_LEAFNODE_ARRAY_TYPE_H_
