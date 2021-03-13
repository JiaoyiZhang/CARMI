/**
 * @file find_function.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_FUNC_FIND_FUNCTION_H_
#define SRC_CARMI_FUNC_FIND_FUNCTION_H_

#include <float.h>

#include <algorithm>

#include "../carmi.h"

CARMI::iterator CARMI::Find(double key) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE: {
        idx = root.lrRoot.childLeft + root.lrRoot.model->Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case PLR_ROOT_NODE: {
        idx = root.plrRoot.childLeft + root.plrRoot.model->Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case HIS_ROOT_NODE: {
        idx = root.hisRoot.childLeft + root.hisRoot.model->Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case BS_ROOT_NODE: {
        idx = root.bsRoot.childLeft + root.bsRoot.model->Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case LR_INNER_NODE: {
        idx = entireChild[idx].lr.childLeft + entireChild[idx].lr.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case PLR_INNER_NODE: {
        idx =
            entireChild[idx].plr.childLeft + entireChild[idx].plr.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case HIS_INNER_NODE: {
        idx =
            entireChild[idx].his.childLeft + entireChild[idx].his.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case BS_INNER_NODE: {
        idx = entireChild[idx].bs.childLeft + entireChild[idx].bs.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case ARRAY_LEAF_NODE: {
        auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].array.Predict(key);
        auto left = entireChild[idx].array.m_left;
        if (entireData[left + preIdx].first == key) {
          return CARMI::iterator(this, &entireChild[idx], preIdx);
        } else {
          int start = std::max(0, preIdx - entireChild[idx].array.error) + left;
          int end =
              std::min(size - 1, preIdx + entireChild[idx].array.error) + left;
          start = std::min(start, end);
          int res;
          if (key <= entireData[start].first) {
            res = ArrayBinarySearch(key, left, start);
          } else if (key <= entireData[end].first) {
            res = ArrayBinarySearch(key, start, end);
          } else {
            res = ArrayBinarySearch(key, end, left + size - 1);
            if (res >= left + size) {
              CARMI::iterator it;
              return it.end();
            }
          }
          if (entireData[res].first == key) {
            return CARMI::iterator(this, &entireChild[idx], res - left);
          }
          CARMI::iterator it;
          return it.end();
        }
      } break;
      case GAPPED_ARRAY_LEAF_NODE: {
        auto left = entireChild[idx].ga.m_left;
        int preIdx = entireChild[idx].ga.Predict(key);
        if (entireData[left + preIdx].first == key) {
          return CARMI::iterator(this, &entireChild[idx], preIdx);
        } else {
          int start = std::max(0, preIdx - entireChild[idx].ga.error) + left;
          int end = std::min(entireChild[idx].ga.maxIndex,
                             preIdx + entireChild[idx].ga.error) +
                    left;
          start = std::min(start, end);

          int res;
          if (entireData[start].first == DBL_MIN) start--;
          if (entireData[end].first == DBL_MIN) end--;
          if (key <= entireData[start].first) {
            res = GABinarySearch(key, left, start);
          } else if (key <= entireData[end].first) {
            res = GABinarySearch(key, start, end);
          } else {
            res = GABinarySearch(key, end, left + entireChild[idx].ga.maxIndex);
            if (res > left + entireChild[idx].ga.maxIndex) {
              CARMI::iterator it;
              return it.end();
            }
          }

          if (entireData[res].first == key) {
            return CARMI::iterator(this, &entireChild[idx], res - left);
          }
          CARMI::iterator it;
          return it.end();
        }
      } break;
      case EXTERNAL_ARRAY_LEAF_NODE: {
        auto size = entireChild[idx].ycsbLeaf.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].ycsbLeaf.Predict(key);
        auto left = entireChild[idx].ycsbLeaf.m_left;
        if (externalData[left + preIdx].first == key) {
          return CARMI::iterator(this, &entireChild[idx], preIdx);
        } else {
          int start =
              std::max(0, preIdx - entireChild[idx].ycsbLeaf.error) + left;
          int end =
              std::min(size - 1, preIdx + entireChild[idx].ycsbLeaf.error) +
              left;
          start = std::min(start, end);
          int res;
          if (key <= externalData[start].first) {
            res = YCSBBinarySearch(key, left, start);
          } else if (key <= externalData[end].first) {
            res = YCSBBinarySearch(key, start, end);
          } else {
            res = YCSBBinarySearch(key, end, left + size - 1);
            if (res >= left + size) {
              CARMI::iterator it;
              return it.end();
            }
          }
          if (externalData[res].first == key) {
            return CARMI::iterator(this, &entireChild[idx], res - left);
          }
          CARMI::iterator it;
          return it.end();
        }
      } break;
    }
  }
}

#endif  // SRC_CARMI_FUNC_FIND_FUNCTION_H_
