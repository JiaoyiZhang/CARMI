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
#ifndef SRC_CARMI_NODES_LEAFNODE_YCSB_LEAF_TYPE_H_
#define SRC_CARMI_NODES_LEAFNODE_YCSB_LEAF_TYPE_H_

#include <vector>

#include "../../../params.h"
#include "../../carmi.h"
#include "./ycsb_leaf.h"

inline int CARMI::UpdateError(YCSBLeaf *ycsb, const int start_idx,
                              const int size) {
  // find: max|pi-yi|
  int maxError = 0, p, d;
  for (int i = start_idx; i < start_idx + size; i++) {
    p = ycsb->Predict(initDataset[i].first);
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
      p = ycsb->Predict(initDataset[i].first);
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
      ycsb->error = e;
    }
  }
  return ycsb->error;
}

inline void CARMI::Train(YCSBLeaf *ycsb, const int start_idx, const int size) {
  if ((ycsb->flagNumber & 0x00FFFFFF) != size) ycsb->flagNumber += size;
  std::vector<double> index;
  int end = start_idx + size;
  for (int i = start_idx; i < end; i++)
    index.push_back(static_cast<double>(i - start_idx) /
                    static_cast<double>(size));

  double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
  for (int i = start_idx; i < end; i++) {
    t1 += initDataset[i].first * initDataset[i].first;
    t2 += initDataset[i].first;
    t3 += initDataset[i].first * index[i - start_idx];
    t4 += index[i - start_idx];
  }
  ycsb->theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
  ycsb->theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
}

inline void CARMI::initYCSB(YCSBLeaf *ycsb, const int start_idx,
                            const int size) {
  ycsb->flagNumber += size;
  ycsb->m_left = start_idx;
  if (size == 0) return;

  Train(ycsb, start_idx, size);
  UpdateError(ycsb, start_idx, size);
}

#endif  // SRC_CARMI_NODES_LEAFNODE_YCSB_LEAF_TYPE_H_
