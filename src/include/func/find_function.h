/**
 * @file find_function.h
 * @author Jiaoyi
 * @brief find a record
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_FUNC_FIND_FUNCTION_H_
#define SRC_INCLUDE_FUNC_FIND_FUNCTION_H_

#include <float.h>

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "./inlineFunction.h"

template <typename KeyType, typename ValueType>
BaseNode<KeyType, ValueType> *CARMI<KeyType, ValueType>::Find(
    const KeyType &key, int *currblock, int *currslot) {
  int idx = 0;
  int type = root.flagNumber;
  int fetch_start = 0;
  double fetch_leafIdx;
  while (1) {
    switch (type) {
      case PLR_ROOT_NODE:
        idx = root.PLRType<DataVectorType, KeyType>::model.Predict(key);
        if (isPrimary == false) {
          fetch_leafIdx =
              root.PLRType<DataVectorType, KeyType>::model.PredictIdx(key);
          fetch_start = root.PLRType<DataVectorType, KeyType>::fetch_model
                            .PrefetchPredict(fetch_leafIdx);
#ifdef Ubuntu
          __builtin_prefetch(&data.dataArray[fetch_start], 0, 3);
          // __builtin_prefetch(&data.dataArray[fetch_start] + 64, 0, 3);
          // __builtin_prefetch(&data.dataArray[fetch_start] + 128, 0, 3);
          // __builtin_prefetch(&data.dataArray[fetch_start] + 192, 0, 3);
#endif
#ifdef Windows
          _mm_prefetch(static_cast<char *>(
                           static_cast<void *>(&data.dataArray[fetch_start])),
                       _MM_HINT_T1);
          _mm_prefetch(static_cast<char *>(
                           static_cast<void *>(&data.dataArray[fetch_start])) +
                           64,
                       _MM_HINT_T1);
          _mm_prefetch(static_cast<char *>(
                           static_cast<void *>(&data.dataArray[fetch_start])) +
                           128,
                       _MM_HINT_T1);
          _mm_prefetch(static_cast<char *>(
                           static_cast<void *>(&data.dataArray[fetch_start])) +
                           192,
                       _MM_HINT_T1);
#endif
        }
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case LR_INNER_NODE:
        idx = node.nodeArray[idx].lr.childLeft +
              node.nodeArray[idx].lr.Predict(key);
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case PLR_INNER_NODE:
        idx = node.nodeArray[idx].plr.childLeft +
              node.nodeArray[idx].plr.Predict(key);
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case HIS_INNER_NODE:
        idx = node.nodeArray[idx].his.childLeft +
              node.nodeArray[idx].his.Predict(key);
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case BS_INNER_NODE:
        idx = node.nodeArray[idx].bs.childLeft +
              node.nodeArray[idx].bs.Predict(key);
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case ARRAY_LEAF_NODE: {
        *currslot = node.nodeArray[idx].cfArray.Find(data, key, currblock);
        return &node.nodeArray[idx];
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        *currslot = node.nodeArray[idx].externalArray.Find(key, recordLength,
                                                           external_data);
        return &node.nodeArray[idx];
      }
    }
  }
}

#endif  // SRC_INCLUDE_FUNC_FIND_FUNCTION_H_
