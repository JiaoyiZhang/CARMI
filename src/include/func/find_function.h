/**
 * @file find_function.h
 * @author Jiaoyi
 * @brief find a record
 * @version 0.1
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

#include "../carmi.h"

template <typename KeyType, typename ValueType>
BaseNode<KeyType>* CARMI<KeyType, ValueType>::Find(KeyType key, int* currunion,
                                                   int* currslot) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  int fetch_start = 0;
  int fetch_size = kPrefetchRange * 2;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE:
        idx = root.childLeft +
              root.LRType<DataVectorType, KeyType>::model.Predict(key);
        fetch_start =
            root.LRType<DataVectorType, KeyType>::fetch_model.Predict(key) -
            kPrefetchRange;
        // std::cout << "fetch_start: " << fetch_start << std::endl;
        // std::cout << "FetchPredict: "
        //           << root.LRType<DataVectorType, KeyType>::fetch_model.Predict(
        //                  key)
        //           << std::endl;
        if (fetch_start < 0) {
          fetch_start = 0;
        }
        if (fetch_size > nowDataSize / carmi_params::kMaxLeafNodeSize) {
          fetch_size = nowDataSize / carmi_params::kMaxLeafNodeSize;
        }
        memcpy(reinterpret_cast<char*>(&BufferPool[0]),
               reinterpret_cast<char*>(&entireData[fetch_start]),
               fetch_size * sizeof(entireData[0]));
        // memcpy(BufferPool, entireData + fetch_start, fetch_size);
        break;
      case LR_INNER_NODE:
        idx = entireChild[idx].lr.childLeft + entireChild[idx].lr.Predict(key);
        break;
      case PLR_INNER_NODE:
        idx =
            entireChild[idx].plr.childLeft + entireChild[idx].plr.Predict(key);
        break;
      case HIS_INNER_NODE:
        idx =
            entireChild[idx].his.childLeft + entireChild[idx].his.Predict(key);
        break;
      case BS_INNER_NODE:
        idx = entireChild[idx].bs.childLeft + entireChild[idx].bs.Predict(key);
        break;
      case ARRAY_LEAF_NODE: {
        // int size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        *currunion = entireChild[idx].array.Predict(key);
        int left = entireChild[idx].array.m_left;
        int find_idx = left + *currunion;
        // std::cout << "find_idx: " << find_idx << std::endl;
        // std::cout << "fetch_start: " << fetch_start << std::endl;
        // std::cout
        //     << "fetch_size / carmi_params::kMaxLeafNodeSize + fetch_start: "
        //     << fetch_size + fetch_start << std::endl;
        // std::cout << std::endl;
        if (find_idx >= fetch_start && find_idx < fetch_size + fetch_start) {
          // access BufferPool
          RebalanceNum++;
          // std::cout << "access buffer pool! " << std::endl;
          find_idx -= fetch_start;
          int res = SlotsUnionSearch(BufferPool[find_idx], key);
          if (BufferPool[find_idx].slots[res].first == key) {
            *currslot = res;
            return &entireChild[idx];
          } else {
            *currslot = -1;
            return &entireChild[idx];
          }
        } else {
          // access entireData
          int res = SlotsUnionSearch(entireData[find_idx], key);
          if (entireData[find_idx].slots[res].first == key) {
            *currslot = res;
            return &entireChild[idx];
          } else {
            *currslot = -1;
            return &entireChild[idx];
          }
        }
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        auto size = entireChild[idx].externalArray.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].externalArray.Predict(key);
        auto left = entireChild[idx].externalArray.m_left;

        if (*reinterpret_cast<const KeyType*>(
                static_cast<const char*>(external_data) +
                (left + preIdx) * recordLength) == key) {
          *currslot = preIdx;
          return &entireChild[idx];
        }

        preIdx = ExternalSearch(
            key, preIdx, entireChild[idx].externalArray.error, left, size);

        if (preIdx >= left + size ||
            *reinterpret_cast<const KeyType*>(
                static_cast<const char*>(external_data) +
                preIdx * recordLength) != key) {
          *currslot = 0;
          return NULL;
        }
        *currslot = preIdx - left;
        return &entireChild[idx];
      }
    }

    type = entireChild[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_INCLUDE_FUNC_FIND_FUNCTION_H_
