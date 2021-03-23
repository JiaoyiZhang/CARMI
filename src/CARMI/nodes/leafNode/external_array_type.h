/**
 * @file ycsb_leaf_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
#define SRC_CARMI_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_

#include <vector>

#include "../../../params.h"
#include "../../carmi.h"
#include "./leaf_nodes.h"

inline void CARMI::Train(int start_idx, int size, const DataVectorType &dataset,
                         ExternalArray *ext) {
  int actualSize = 0;
  std::vector<float> index(size, 0);
  int end = start_idx + size;
  for (int i = start_idx, j = 0; i < end; i++, j++) {
    if (dataset[i].first != DBL_MIN) {
      actualSize++;
    }
    index[j] = static_cast<float>(i - start_idx) / size;
  }
  if (actualSize == 0) return;
  if ((ext->flagNumber & 0x00FFFFFF) != actualSize) {
    ext->flagNumber += actualSize;
  }

  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = start_idx; i < end; i++) {
    if (dataset[i].first != DBL_MIN) {
      t1 += dataset[i].first * dataset[i].first;
      t2 += dataset[i].first;
      t3 += dataset[i].first * index[i - start_idx];
      t4 += index[i - start_idx];
    }
  }
  ext->theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
  ext->theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);

  // find: max|pi-yi|
  int maxError = 0, p, d;
  for (int i = start_idx; i < start_idx + size; i++) {
    if (dataset[i].first != DBL_MIN) {
      p = ext->Predict(dataset[i].first);
      d = abs(i - start_idx - p);
      if (d > maxError) maxError = d;
    }
  }

  // find the optimal value of error
  int minRes = size * log2(size);
  int res;
  int cntBetween, cntOut;
  for (int e = 0; e <= maxError; e++) {
    cntBetween = 0;
    cntOut = 0;
    for (int i = start_idx; i < start_idx + size; i++) {
      if (dataset[i].first != DBL_MIN) {
        p = ext->Predict(dataset[i].first);
        d = abs(i - start_idx - p);
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
      ext->error = e;
    }
  }
}

inline void CARMI::InitExternalArray(int start_idx, int size,
                                     const DataVectorType &dataset,
                                     ExternalArray *ext) {
  ext->m_left = start_idx;
  if (size == 0) return;

  Train(start_idx, size, dataset, ext);
}

#endif  // SRC_CARMI_NODES_LEAFNODE_EXTERNAL_ARRAY_TYPE_H_
