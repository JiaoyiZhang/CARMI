/**
 * @file delete_function.h
 * @author Jiaoyi
 * @brief delete a record
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_FUNC_DELETE_FUNCTION_H_
#define SRC_INCLUDE_FUNC_DELETE_FUNCTION_H_

#include <algorithm>

#include "../carmi.h"
#include "inlineFunction.h"

template <typename KeyType, typename ValueType>
bool CARMI<KeyType, ValueType>::Delete(KeyType key) {
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
        auto left = entireChild[idx].array.m_left;
        auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        left += entireChild[idx].array.Predict(key);

        int res = SlotsUnionSearch(entireData[left], key);
        if (entireData[left].slots[res].first == key) {
          int i = res;
          for (; i < kMaxSlotNum - 1; i++) {
            entireData[left].slots[i] = entireData[left].slots[i + 1];
            if (entireData[left].slots[i + 1].first == DBL_MIN) {
              break;
            }
          }
          entireData[left].slots[i] = {DBL_MIN, DBL_MIN};
          return true;
        }
        return false;
      }
    }

    type = entireChild[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_INCLUDE_FUNC_DELETE_FUNCTION_H_
