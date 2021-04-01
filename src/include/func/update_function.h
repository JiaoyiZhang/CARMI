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
/**
 * @brief update a record of the given key
 *
 * @param data
 * @return true if the operation succeeds
 * @return false if the operation fails
 */
template <typename KeyType, typename ValueType>
bool CARMI<KeyType, ValueType>::Update(DataType data) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE:
        idx = root.childLeft + root.LRType::model.Predict(data.first);
        break;
      case PLR_ROOT_NODE:
        idx = root.childLeft + root.PLRType::model.Predict(data.first);
        break;
      case HIS_ROOT_NODE:
        idx = root.childLeft + root.HisType::model.Predict(data.first);
        break;
      case BS_ROOT_NODE:
        idx = root.childLeft + root.BSType::model.Predict(data.first);
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
        int preIdx = entireChild[idx].array.Predict(data.first);
        if (entireData[left + preIdx].first == data.first) {
          entireData[left + preIdx].second = data.second;
        } else {
          preIdx = ArraySearch(data.first, preIdx, entireChild[idx].array.error,
                               left, size);
          if (preIdx >= left + size || entireData[preIdx].first != data.first)
            return false;

          entireData[preIdx].second = data.second;
        }
        return true;
      }
      case GAPPED_ARRAY_LEAF_NODE: {
        auto left = entireChild[idx].ga.m_left;
        int preIdx = entireChild[idx].ga.Predict(data.first);
        int maxIndex = entireChild[idx].ga.maxIndex;
        if (entireData[left + preIdx].first == data.first) {
          entireData[left + preIdx].second = data.second;
          return true;
        } else {
          preIdx = GASearch(data.first, preIdx, entireChild[idx].ga.error, left,
                            maxIndex);

          if (preIdx > left + maxIndex ||
              entireData[preIdx].first != data.first)
            return false;

          entireData[preIdx].second = data.second;
          return true;
        }
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        auto size = entireChild[idx].externalArray.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].externalArray.Predict(data.first);
        auto left = entireChild[idx].externalArray.m_left;
        if (externalData[left + preIdx].first == data.first) {
          externalData[left + preIdx].second = data.second;
          return true;
        } else {
          preIdx =
              ExternalSearch(data.first, preIdx,
                             entireChild[idx].externalArray.error, left, size);
          if (preIdx >= left + size || externalData[preIdx].first != data.first)
            return false;

          externalData[preIdx].second = data.second;
          return true;
        }
      }
    }

    type = entireChild[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_INCLUDE_FUNC_UPDATE_FUNCTION_H_
