/**
 * @file update_function.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_FUNC_UPDATE_FUNCTION_H_
#define SRC_CARMI_FUNC_UPDATE_FUNCTION_H_

#include <algorithm>
#include <utility>

#include "../carmi.h"
#include "inlineFunction.h"

bool CARMI::Update(DataType data) {
  int idx = 0;  // idx in the INDEX
  int content;
  int type = rootType;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE: {
        idx = root.lrRoot.childLeft + root.lrRoot.model.Predict(data.first);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case PLR_ROOT_NODE: {
        idx = root.plrRoot.childLeft + root.plrRoot.model.Predict(data.first);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case HIS_ROOT_NODE: {
        idx = root.hisRoot.childLeft + root.hisRoot.model.Predict(data.first);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case BS_ROOT_NODE: {
        idx = root.bsRoot.childLeft + root.bsRoot.model.Predict(data.first);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case LR_INNER_NODE: {
        idx = entireChild[idx].lr.childLeft +
              entireChild[idx].lr.Predict(data.first);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case PLR_INNER_NODE: {
        idx = entireChild[idx].plr.childLeft +
              entireChild[idx].plr.Predict(data.first);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case HIS_INNER_NODE: {
        idx = entireChild[idx].his.childLeft +
              entireChild[idx].his.Predict(data.first);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case BS_INNER_NODE: {
        idx = entireChild[idx].bs.childLeft +
              entireChild[idx].bs.Predict(data.first);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case ARRAY_LEAF_NODE: {
        auto left = entireChild[idx].array.m_left;
        auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].array.Predict(data.first);
        if (entireData[left + preIdx].first == data.first) {
          entireData[left + preIdx].second = data.second;
        } else {
          int start = std::max(0, preIdx - entireChild[idx].array.error) + left;
          int end =
              std::min(size - 1, preIdx + entireChild[idx].array.error) + left;
          start = std::min(start, end);
          if (data.first <= entireData[start].first) {
            preIdx = ArrayBinarySearch(data.first, left, start);
          } else if (data.first <= entireData[end].first) {
            preIdx = ArrayBinarySearch(data.first, start, end);
          } else {
            preIdx = ArrayBinarySearch(data.first, end, left + size - 1);
            if (preIdx >= left + size) return false;
          }
          if (entireData[preIdx].first != data.first) return false;
          entireData[preIdx].second = data.second;
        }
        return true;
      } break;
      case GAPPED_ARRAY_LEAF_NODE: {
        auto left = entireChild[idx].ga.m_left;
        int preIdx = entireChild[idx].ga.Predict(data.first);
        if (entireData[left + preIdx].first == data.first) {
          entireData[left + preIdx].second = data.second;
          return true;
        } else {
          int start = std::max(0, preIdx - entireChild[idx].ga.error) + left;
          int end = std::min(entireChild[idx].ga.maxIndex,
                             preIdx + entireChild[idx].ga.error) +
                    left;
          start = std::min(start, end);
          if (entireData[start].first == -1) start--;
          if (entireData[end].first == -1) end--;

          if (data.first <= entireData[start].first) {
            preIdx = GABinarySearch(data.first, left, start);
          } else if (data.first <= entireData[end].first) {
            preIdx = GABinarySearch(data.first, start, end);
          } else {
            preIdx = GABinarySearch(data.first, end,
                                    left + entireChild[idx].ga.maxIndex);
            if (preIdx > left + entireChild[idx].ga.maxIndex) return false;
          }

          if (entireData[preIdx].first != data.first) return false;
          entireData[preIdx].second = data.second;
          return true;
        }
      } break;
      case EXTERNAL_ARRAY_LEAF_NODE: {
        auto size = entireChild[idx].ycsbLeaf.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].ycsbLeaf.Predict(data.first);
        auto left = entireChild[idx].ycsbLeaf.m_left;
        if (externalData[left + preIdx].first == data.first) {
          externalData[left + preIdx].second = data.second;
          return true;
        } else {
          int start =
              std::max(0, preIdx - entireChild[idx].ycsbLeaf.error) + left;
          int end =
              std::min(size - 1, preIdx + entireChild[idx].ycsbLeaf.error) +
              left;
          start = std::min(start, end);
          int res;
          if (data.first <= externalData[start].first) {
            res = YCSBBinarySearch(data.first, left, start);
          } else if (data.first <= externalData[end].first) {
            res = YCSBBinarySearch(data.first, start, end);
          } else {
            res = YCSBBinarySearch(data.first, end, left + size - 1);
            if (res >= left + size) {
              return false;
            }
          }
          if (externalData[res].first == data.first) {
            externalData[res].second = data.second;
            return true;
          }
          return false;
        }
      } break;
    }
  }
}

#endif  // SRC_CARMI_FUNC_UPDATE_FUNCTION_H_
