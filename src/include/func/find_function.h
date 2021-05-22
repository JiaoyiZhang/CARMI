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

#include "../carmi.h"

template <typename KeyType, typename ValueType>
BaseNode* CARMI<KeyType, ValueType>::Find(KeyType key, int* currslot) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE:
        idx = root.childLeft +
              root.LRType<DataVectorType, DataType>::model.Predict(key);
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
        int size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].array.Predict(key);
        int left = entireChild[idx].array.m_left;

        if (entireData[left + preIdx].first == key) {
          *currslot = preIdx;
          return &entireChild[idx];
        }

        preIdx =
            ArraySearch(key, preIdx, entireChild[idx].array.error, left, size);
        if (preIdx >= left + size || entireData[preIdx].first != key) {
          *currslot = 0;
          return NULL;
        }
        *currslot = preIdx - left;
        return &entireChild[idx];
      }
      case GAPPED_ARRAY_LEAF_NODE: {
        int left = entireChild[idx].ga.m_left;
        int maxIndex = entireChild[idx].ga.maxIndex;
        int preIdx = entireChild[idx].ga.Predict(key);

        if (entireData[left + preIdx].first == key) {
          *currslot = preIdx;
          return &entireChild[idx];
        }

        preIdx =
            GASearch(key, preIdx, entireChild[idx].ga.error, left, maxIndex);
        if (preIdx > left + maxIndex || entireData[preIdx].first != key) {
          *currslot = 0;
          return NULL;
        }

        *currslot = preIdx - left;
        return &entireChild[idx];
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
