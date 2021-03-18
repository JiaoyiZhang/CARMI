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

inline void CARMI::InitArray(int cap, int left, int size,
                             const DataVectorType &dataset, ArrayType *arr) {
  if (size == 0) return;
  DataVectorType newDataset(size, {DBL_MIN, DBL_MIN});
  int actualSize = 0;
  for (int i = left; i < left + size; i++) {
    if (dataset[i].second != DBL_MIN) {
      newDataset[actualSize++] = dataset[i];
    }
  }

  StoreData(cap, left, size, newDataset, arr);

  if (size > kLeafMaxCapacity)
    std::cout << "init Array setDataset WRONG! datasetSize > 4096, size is:"
              << size << std::endl;

  Train(arr->m_left, size, entireData, arr);
}

inline void CARMI::StoreData(int cap, int left, int size,
                             const DataVectorType &dataset, ArrayType *arr) {
  if (arr->m_left != -1) {
    ReleaseMemory(arr->m_left, arr->m_capacity);
  }
  if (cap < size) {
    arr->m_capacity = GetIndex(size);
  } else {
    arr->m_capacity = cap;
  }
  arr->flagNumber += size;

  arr->m_left = AllocateMemory(arr->m_capacity);

  int end = left + size;
  for (int i = arr->m_left, j = left; j < end; i++, j++)
    entireData[i] = dataset[j];
}

inline void CARMI::Train(int start_idx, int size, const DataVectorType &dataset,
                         ArrayType *arr) {
  int actualSize = 0;
  std::vector<double> index;
  int end = start_idx + size;
  for (int i = start_idx; i < end; i++) {
    if (dataset[i].first != DBL_MIN) {
      actualSize++;
    }
    index.push_back(static_cast<double>(i - start_idx) / size);
  }
  if (actualSize == 0) return;

  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = start_idx; i < end; i++) {
    if (dataset[i].first != DBL_MIN) {
      t1 += dataset[i].first * dataset[i].first;
      t2 += dataset[i].first;
      t3 += dataset[i].first * index[i - start_idx];
      t4 += index[i - start_idx];
    }
  }
  arr->theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
  arr->theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);

  // find: max|pi-yi|
  int maxError = 0, p, d;
  for (int i = start_idx; i < end; i++) {
    p = arr->Predict(dataset[i].first);
    d = abs(i - start_idx - p);
    if (d > maxError) maxError = d;
  }

  // find the optimal value of error
  int minRes = size * log2(size);
  int res;
  int cntBetween, cntOut;
  for (int e = 0; e <= maxError; e++) {
    cntBetween = 0;
    cntOut = 0;
    for (int i = start_idx; i < start_idx + size; i++) {
      p = arr->Predict(dataset[i].first);
      d = abs(i - start_idx - p);
      if (d <= e)
        cntBetween++;
      else
        cntOut++;
    }
    if (e != 0)
      res = cntBetween * log2(e) + cntOut * log2(size);
    else
      res = cntOut * log2(size);
    if (res < minRes) {
      minRes = res;
      arr->error = e;
    }
  }
}

inline void CARMI::Train(int start_idx, int size, ArrayType *arr) {
  int actualSize = 0;
  std::vector<double> index;
  int end = start_idx + size;
  for (int i = start_idx; i < end; i++) {
    if (initDataset[i].first != DBL_MIN) {
      actualSize++;
    }
    index.push_back(static_cast<double>(i - start_idx) / size);
  }
  if (actualSize == 0) return;

  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = start_idx; i < end; i++) {
    if (initDataset[i].first != DBL_MIN) {
      t1 += initDataset[i].first * initDataset[i].first;
      t2 += initDataset[i].first;
      t3 += initDataset[i].first * index[i - start_idx];
      t4 += index[i - start_idx];
    }
  }
  arr->theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
  arr->theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);

  // find: max|pi-yi|
  int maxError = 0, p, d;
  for (int i = start_idx; i < end; i++) {
    p = arr->Predict(initDataset[i].first);
    d = abs(i - start_idx - p);
    if (d > maxError) maxError = d;
  }

  // find the optimal value of error
  int minRes = size * log2(size);
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
      res = cntBetween * log2(e) + cntOut * log2(size);
    else
      res = cntOut * log2(size);
    if (res < minRes) {
      minRes = res;
      arr->error = e;
    }
  }
}
#endif  // SRC_CARMI_NODES_LEAFNODE_ARRAY_TYPE_H_
