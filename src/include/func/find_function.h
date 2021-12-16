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
#ifndef FUNC_FIND_FUNCTION_H_
#define FUNC_FIND_FUNCTION_H_

#include <float.h>

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

#if defined(CATCH_PLATFORM_WINDOWS)
#include <xmmintrin.h>
#endif

#include "../carmi.h"

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
BaseNode<KeyType, ValueType, Compare, Alloc> *
CARMI<KeyType, ValueType, Compare, Alloc>::Find(const KeyType &key,
                                                int *currblock, int *currslot) {
  int idx = 0;
  int type = root.flagNumber;
  int fetch_start = 0;
  double fetch_leafIdx;
  while (1) {
    switch (type) {
      case PLR_ROOT_NODE:
        // Case 0: this node is the plr root node
        // use the plr root node to find the index of the next node and prefetch
        // the data block
        if (isPrimary == false) {
          fetch_leafIdx =
              root.PLRType<DataVectorType, KeyType>::model.Predict(key);
          idx = fetch_leafIdx;
          fetch_start = root.PLRType<DataVectorType, KeyType>::fetch_model
                            .PrefetchPredict(fetch_leafIdx);
#if defined(CATCH_PLATFORM_LINUX) || defined(CATCH_PLATFORM_MAC)
          // the instructions of prefetching in Ubuntu
          __builtin_prefetch(&data.dataArray[fetch_start], 0, 3);
          // __builtin_prefetch(&data.dataArray[fetch_start] + 64, 0, 3);
          // __builtin_prefetch(&data.dataArray[fetch_start] + 128, 0, 3);
          // __builtin_prefetch(&data.dataArray[fetch_start] + 192, 0, 3);
#elif defined(CATCH_PLATFORM_WINDOWS)
          // the instructions of prefetching in Windows
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
        } else {
          idx = root.PLRType<DataVectorType, KeyType>::model.Predict(key);
        }
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case LR_INNER_NODE:
        // Case 1: this node is the lr inner node
        // use the predict function of lr inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].lr.Predict(key);
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case PLR_INNER_NODE:
        // Case 2: this node is the plr inner node
        // use the predict function of plr inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].plr.Predict(key);
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case HIS_INNER_NODE:
        // Case 3: this node is the his inner node
        // use the predict function of his inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].his.Predict(key);
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case BS_INNER_NODE:
        // Case 4: this node is the bs inner node
        // use the predict function of bs inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].bs.Predict(key);
        type = node.nodeArray[idx].lr.flagNumber >> 24;
        break;
      case ARRAY_LEAF_NODE: {
        // Case 5: this node is the cache-friendly array leaf node
        // find the data point in the cf leaf node and return its position
        *currslot = node.nodeArray[idx].cfArray.Find(data, key, currblock);
        return &node.nodeArray[idx];
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        // Case 6: this node is the external array leaf node
        // find the data point in the external leaf node and return its position
        *currslot = node.nodeArray[idx].externalArray.Find(key, recordLength,
                                                           external_data);
        return &node.nodeArray[idx];
      }
    }
  }
}

#endif  // FUNC_FIND_FUNCTION_H_
