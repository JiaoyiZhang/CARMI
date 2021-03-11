/**
 * @file plr_model.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_INNERNODE_PLR_MODEL_H_
#define SRC_CARMI_NODES_INNERNODE_PLR_MODEL_H_

#include <float.h>

#include <utility>
#include <vector>

#include "../../carmi.h"

inline void CARMI::initPLR(const DataVectorType &dataset, PLRModel *plr) {
  int childNumber = plr->flagNumber & 0x00FFFFFF;
  plr->childLeft = allocateChildMemory(childNumber);
  if (dataset.size() == 0) return;

  Train(0, dataset.size(), dataset, plr);

  std::vector<DataVectorType> perSubDataset;
  DataVectorType tmp;
  for (int i = 0; i < childNumber; i++) perSubDataset.push_back(tmp);
  for (int i = 0; i < dataset.size(); i++) {
    int p = plr->Predict(dataset[i].first);
    perSubDataset[p].push_back(dataset[i]);
  }

  switch (kLeafNodeID) {
    case ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        ArrayType tmp(kThreshold);
        initArray(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i],
                  &tmp);
        entireChild[plr->childLeft + i].array = tmp;
      }
      break;
    case GAPPED_ARRAY_LEAF_NODE:
      for (int i = 0; i < childNumber; i++) {
        GappedArrayType tmp(kThreshold);
        initGA(kMaxKeyNum, 0, perSubDataset[i].size(), perSubDataset[i], &tmp);
        entireChild[plr->childLeft + i].ga = tmp;
      }
      break;
  }
}

inline void CARMI::Train(const int left, const int size,
                         const DataVectorType &dataset, PLRModel *plr) {
  int childNumber = plr->flagNumber & 0x00FFFFFF;
  int actualSize = 0;
  std::vector<double> xx(7, 0.0);
  std::vector<double> x(7, 0.0);
  std::vector<double> px(7, 0.0);
  std::vector<double> pp(7, 0.0);
  if (size == 0) return;
  int width = size / 7;
  for (int i = 0; i < 7; i++) {
    plr->point[i] = {dataset[(i + 1) * width].first, childNumber * (i + 1) / 7};
  }
  if (childNumber <= 32) return;
  for (int k = 0; k < 7; k++) {
    int start = k * width;
    int end = (k + 1) * width;
    for (int i = start; i < end; i++) {
      xx[k] += dataset[i].first * dataset[i].first;
      x[k] += dataset[i].first;
      px[k] += static_cast<float>(i) / size * dataset[i].first;
      pp[k] += static_cast<float>(i * i) / size / size;
    }
    px[k] *= childNumber;
    pp[k] *= childNumber * childNumber;
  }
  double opt = DBL_MAX;
  DataVectorType theta(6, {1, 1});
  int bound = childNumber / 7;
  int a, b;
  for (int t1 = -2; t1 < 2; t1++) {
    theta[0] = {static_cast<float>(bound + t1) / plr->point[0].first, 0};
    for (int t2 = -2; t2 < 2; t2++) {
      a = static_cast<float>(bound + t2 - t1) /
          (plr->point[1].first - plr->point[0].first);
      b = bound * 2 + t2 - a * plr->point[1].first;
      theta[1] = {a, b};
      for (int t3 = -2; t3 < 2; t3++) {
        a = static_cast<float>(bound + t3 - t2) /
            (plr->point[2].first - plr->point[1].first);
        b = bound * 3 + t3 - a * plr->point[2].first;
        theta[2] = {a, b};
        for (int t4 = -2; t4 < 2; t4++) {
          a = static_cast<float>(bound + t4 - t3) /
              (plr->point[3].first - plr->point[2].first);
          b = bound * 4 + t4 - a * plr->point[3].first;
          theta[3] = {a, b};
          for (int t5 = -2; t5 < 2; t5++) {
            a = static_cast<float>(bound + t5 - t4) /
                (plr->point[4].first - plr->point[3].first);
            b = bound * 5 + t5 - a * plr->point[4].first;
            theta[4] = {a, b};
            for (int t6 = -2; t6 < 2; t6++) {
              a = static_cast<float>(bound + t6 - t5) /
                  (plr->point[5].first - plr->point[4].first);
              b = bound * 6 + t6 - a * plr->point[5].first;
              theta[5] = {a, b};
              a = static_cast<float>(childNumber - 6 * bound - t6) /
                  (plr->maxX - plr->point[5].first);
              b = childNumber - a * plr->maxX;
              theta[6] = {a, b};

              double value = 0.0;
              for (int i = 0; i < 7; i++) {
                value += theta[i].first * theta[i].first * xx[i];
                value += 2 * theta[i].first * theta[i].second * x[i];
                value -= 2 * (theta[i].first + theta[i].second) * px[i];
                value += theta[i].second * theta[i].second;
                value += pp[i];
              }
              if (value < opt) {
                opt = value;
                plr->point[0].second = bound + t1;
                plr->point[1].second = 2 * bound + t2;
                plr->point[2].second = 3 * bound + t3;
                plr->point[3].second = 4 * bound + t4;
                plr->point[4].second = 5 * bound + t5;
                plr->point[5].second = 6 * bound + t6;
              }
            }
          }
        }
      }
    }
  }
}

inline int PLRModel::Predict(double key) const {
  int s = 0;
  int e = 5;
  int mid;
  while (s < e) {
    mid = (s + e) / 2;
    if (point[mid].first < key)
      s = mid + 1;
    else
      e = mid;
  }

  int p;
  if (s == 0) {
    p = static_cast<float>(point[0].second) / point[0].first * key;
  } else if (s == 5) {
    int childNumber = flagNumber & 0x00FFFFFF;
    int a = static_cast<float>(childNumber - point[5].second) /
            (maxX - point[5].first);
    int b = childNumber - a * maxX;
    p = a * key + b;
  } else {
    int a = static_cast<float>(point[s].second - point[s - 1].second) /
            (point[s].second - point[s - 1].first);
    int b = point[s].second - a * point[s].first;
    p = a * key + b;
  }
  if (p >= (flagNumber & 0x00FFFFFF)) p = (flagNumber & 0x00FFFFFF) - 1;

  return p;
}

#endif  // SRC_CARMI_NODES_INNERNODE_PLR_MODEL_H_
