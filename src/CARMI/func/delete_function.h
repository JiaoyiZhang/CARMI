/**
 * @file delete_function.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_FUNC_DELETE_FUNCTION_H_
#define SRC_CARMI_FUNC_DELETE_FUNCTION_H_

#include <algorithm>

#include "../carmi.h"
#include "inlineFunction.h"

bool CARMI::Delete(double key) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE:
        idx = root.childLeft + root.LRType::model.Predict(key);
        break;
      case PLR_ROOT_NODE:
        idx = root.childLeft + root.PLRType::model.Predict(key);
        break;
      case HIS_ROOT_NODE:
        idx = root.childLeft + root.HisType::model.Predict(key);
        break;
      case BS_ROOT_NODE:
        idx = root.childLeft + root.BSType::model.Predict(key);
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
        int preIdx = entireChild[idx].array.Predict(key);
        if (entireData[left + preIdx].first == key)
          preIdx += left;
        else
          preIdx = ArraySearch(key, preIdx, entireChild[idx].array.error, left,
                               size);
        if (preIdx >= left + size || entireData[preIdx].first != key)
          return false;

        for (int i = preIdx; i < left + size - 1; i++)
          entireData[i] = entireData[i + 1];
        entireData[left + size - 1] = {DBL_MIN, DBL_MIN};
        entireChild[idx].array.flagNumber--;
        return true;
      }
      case GAPPED_ARRAY_LEAF_NODE: {
        // DBL_MIN means the data has been deleted
        // when a data has been deleted, data.second == DBL_MIN
        int left = entireChild[idx].ga.m_left;
        int preIdx = entireChild[idx].ga.Predict(key);
        int maxIndex = entireChild[idx].ga.maxIndex;
        if (entireData[left + preIdx].first == key) {
          entireData[left + preIdx].second = DBL_MIN;
          entireChild[idx].ga.flagNumber--;
          if (preIdx == maxIndex) entireChild[idx].ga.maxIndex--;
          return true;
        } else {
          preIdx =
              GASearch(key, preIdx, entireChild[idx].ga.error, left, maxIndex);

          if (preIdx > left + maxIndex || entireData[preIdx].first != key)
            return false;

          entireChild[idx].ga.flagNumber--;
          entireData[preIdx] = {DBL_MIN, DBL_MIN};
          if (preIdx == left + maxIndex) entireChild[idx].ga.maxIndex--;
          return true;
        }
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        auto size = entireChild[idx].externalArray.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].externalArray.Predict(key);
        auto left = entireChild[idx].externalArray.m_left;
        if (externalData[left + preIdx].first == key) {
          externalData.erase(externalData.begin() + left + preIdx);
          return true;
        } else {
          preIdx = ExternalSearch(
              key, preIdx, entireChild[idx].externalArray.error, left, size);

          if (preIdx >= left + size || externalData[preIdx].first != key)
            return false;

          externalData.erase(externalData.begin() + preIdx);
          return true;
        }
      }
    }

    type = entireChild[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_CARMI_FUNC_DELETE_FUNCTION_H_
