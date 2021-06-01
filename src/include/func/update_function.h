/**
 * @file update_function.h
 * @author Jiaoyi
 * @brief update a record
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_FUNC_UPDATE_FUNCTION_H_
#define SRC_INCLUDE_FUNC_UPDATE_FUNCTION_H_

#include <algorithm>
#include <utility>

#include "../carmi.h"
#include "inlineFunction.h"

template <typename KeyType, typename ValueType>
bool CARMI<KeyType, ValueType>::Update(DataType data) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE:
        idx = root.childLeft +
              root.LRType<DataVectorType, DataType>::model.Predict(data.first);
        break;
      case LR_INNER_NODE:
        idx = entireChild[idx].lr.childLeft +
              entireChild[idx].lr.Predict(data.first);
        break;
      case PLR_INNER_NODE:
        idx = entireChild[idx].plr.childLeft +
              entireChild[idx].plr.Predict(data.first);
        break;
      case HIS_INNER_NODE:
        idx = entireChild[idx].his.childLeft +
              entireChild[idx].his.Predict(data.first);
        break;
      case BS_INNER_NODE:
        idx = entireChild[idx].bs.childLeft +
              entireChild[idx].bs.Predict(data.first);
        break;
      case ARRAY_LEAF_NODE: {
        auto left = entireChild[idx].array.m_left;
        auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        left += entireChild[idx].array.Predict(data.first);

        int res = SlotsUnionSearch(entireData[left], data.first);
        if (entireData[left].slots[res].first == data.first) {
          entireData[left].slots[res].second = data.second;
          return true;
        }
        return false;
      }
    }

    type = entireChild[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_INCLUDE_FUNC_UPDATE_FUNCTION_H_
