/**
 * @file ga_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_LEAFNODE_GA_TYPE_H_
#define SRC_CARMI_NODES_LEAFNODE_GA_TYPE_H_

#include <float.h>

#include <utility>
#include <vector>

#include "../../carmi.h"
#include "./leaf_nodes.h"

inline int GappedArrayType::Predict(double key) const {
  // return the predicted idx in the leaf node
  int p = (theta1 * key + theta2) * maxIndex;
  if (p < 0)
    p = 0;
  else if (p >= maxIndex)
    p = maxIndex;
  return p;
}

inline void CARMI::InitGA(int cap, int left, int size,
                          const DataVectorType &subDataset,
                          GappedArrayType *ga) {
  DataVectorType newDataset(size, {DBL_MIN, DBL_MIN});
  int actualSize = 0;
  for (int i = left; i < left + size; i++) {
    if (subDataset[i].second != DBL_MIN) {
      newDataset[actualSize++] = subDataset[i];
    }
  }
  StoreData(cap, left, size, newDataset, ga);

#ifdef DEBUG
  if (size > kLeafMaxCapacity)
    std::cout << "Gapped Array setDataset WRONG! datasetSize > 4096, size is:"
              << size << std::endl;
#endif  // DEBUG

  Train(ga->m_left, ga->maxIndex, entireData, ga);
}

inline void CARMI::StoreData(int cap, int left, int size,
                             const DataVectorType &dataset,
                             GappedArrayType *ga) {
  if (ga->m_left != -1) {
    ReleaseMemory(ga->m_left, ga->capacity);
  }
  if ((static_cast<float>(size) / cap > ga->density))
    ga->capacity = static_cast<float>(cap) / ga->density;
  if (ga->capacity > kLeafMaxCapacity) {
    ga->capacity = kLeafMaxCapacity;
  }
  ga->capacity = GetActualSize(ga->capacity);
  ga->m_left = AllocateMemory(ga->capacity);

  int k = ga->density / (1 - ga->density);
  float rate = static_cast<float>(dataset.size()) / ga->capacity;
  if (rate > ga->density) {
    k = rate / (1 - rate);
  }
  int cnt = 0;
  int j = ga->m_left;
  int end = left + size;
  for (int i = left; i < end; i++) {
    if (cnt >= k) {
      j++;
      cnt = 0;
      if (j > 2048) k += 2;
    }
    cnt++;
    entireData[j++] = dataset[i];
    ga->maxIndex = j - 1 - ga->m_left;
  }
}

inline void CARMI::Train(int start_idx, int size, const DataVectorType &dataset,
                         GappedArrayType *ga) {
  if ((ga->flagNumber & 0x00FFFFFF) != size) {
    ga->flagNumber = (GAPPED_ARRAY_LEAF_NODE << 24) + size;
    ga->maxIndex = size;
  }
  int actualSize = 0;
  std::vector<float> index(size, 0);
  for (int i = start_idx, j = 0; i < start_idx + size; i++, j++) {
    if (dataset[i].first != DBL_MIN) {
      actualSize++;
    }
    index[j] = static_cast<float>(i - start_idx) / size;
  }
  if (actualSize == 0) {
    return;
  }

  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = start_idx; i < start_idx + size; i++) {
    if (dataset[i].first != DBL_MIN) {
      t1 += dataset[i].first * dataset[i].first;
      t2 += dataset[i].first;
      t3 += dataset[i].first * index[i - start_idx];
      t4 += index[i - start_idx];
    }
  }
  ga->theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
  ga->theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
  ga->maxIndex = size;

  // find: max|pi-yi|
  int maxError = 0, p, d;
  int end = start_idx + size;
  for (int i = start_idx; i < end; i++) {
    if (dataset[i].first != DBL_MIN) {
      p = ga->Predict(dataset[i].first);
      d = abs(i - start_idx - p);
      if (d > maxError) maxError = d;
    }
  }

  // find the optimal value of ga->error
  int minRes = size * log2(size);
  int res;
  int cntBetween, cntOut;
  for (int e = 0; e <= maxError; e++) {
    cntBetween = 0;
    cntOut = 0;
    for (int i = start_idx; i < end; i++) {
      if (dataset[i].first != DBL_MIN) {
        p = ga->Predict(dataset[i].first);
        d = abs(i - p - start_idx);
        if (d <= e)
          cntBetween++;
        else
          cntOut++;
      }
    }
    if (e != 0)
      res = cntBetween * log2(e) + cntOut * log2(size);
    else
      res = cntOut * log2(size);
    if (res < minRes) {
      minRes = res;
      ga->error = e;
    }
  }
}

#endif  // SRC_CARMI_NODES_LEAFNODE_GA_TYPE_H_
