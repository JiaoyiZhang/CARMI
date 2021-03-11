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
#ifndef DELETE_FUNCTION_H
#define DELETE_FUNCTION_H

#include "../carmi.h"
#include "inlineFunction.h"
using namespace std;

bool CARMI::Delete(double key) {
  int idx = 0;  // idx in the INDEX
  int content;
  int type = rootType;
  while (1) {
    switch (type) {
      case 0: {
        idx = root.lrRoot.childLeft + root.lrRoot.model.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case 1: {
        idx = root.plrRoot.childLeft + root.plrRoot.model.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case 2: {
        idx = root.hisRoot.childLeft + root.hisRoot.model.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case 3: {
        idx = root.bsRoot.childLeft + root.bsRoot.model.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case 4: {
        idx = entireChild[idx].lr.childLeft + entireChild[idx].lr.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case 5: {
        idx =
            entireChild[idx].plr.childLeft + entireChild[idx].plr.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case 6: {
        idx =
            entireChild[idx].his.childLeft + entireChild[idx].his.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case 7: {
        idx = entireChild[idx].bs.childLeft + entireChild[idx].bs.Predict(key);
        type = entireChild[idx].lr.flagNumber >> 24;
      } break;
      case 8: {
        auto left = entireChild[idx].array.m_left;
        auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].array.Predict(key);
        if (entireData[left + preIdx].first == key)
          preIdx += left;
        else {
          int start = max(0, preIdx - entireChild[idx].array.error) + left;
          int end = min(size - 1, preIdx + entireChild[idx].array.error) + left;
          start = min(start, end);
          int res;
          if (key <= entireData[start].first)
            res = ArrayBinarySearch(key, left, start);
          else if (key <= entireData[end].first)
            res = ArrayBinarySearch(key, start, end);
          else {
            res = ArrayBinarySearch(key, end, left + size - 1);
            if (res >= left + size) return false;
          }
          if (entireData[res].first == key)
            preIdx = res;
          else
            return false;
        }
        for (int i = preIdx; i < left + size - 1; i++)
          entireData[i] = entireData[i + 1];
        entireData[left + size - 1] = {DBL_MIN, DBL_MIN};
        entireChild[idx].array.flagNumber--;
        return true;
      } break;
      case 9: {
        // DBL_MIN means the data has been deleted
        // when a data has been deleted, data.second == DBL_MIN
        auto left = entireChild[idx].ga.m_left;
        int preIdx = entireChild[idx].ga.Predict(key);
        if (entireData[left + preIdx].first == key) {
          entireData[left + preIdx].second = DBL_MIN;
          entireChild[idx].ga.flagNumber--;
          if (preIdx == entireChild[idx].ga.maxIndex)
            entireChild[idx].ga.maxIndex--;
          return true;
        } else {
          int start = max(0, preIdx - entireChild[idx].ga.error) + left;
          int end = min(entireChild[idx].ga.maxIndex,
                        preIdx + entireChild[idx].ga.error) +
                    left;
          start = min(start, end);

          int res;
          if (entireData[start].first == -1) start--;
          if (entireData[end].first == -1) end--;
          if (key <= entireData[start].first)
            res = GABinarySearch(key, left, start);
          else if (key <= entireData[end].first)
            res = GABinarySearch(key, start, end);
          else {
            res = GABinarySearch(key, end, left + entireChild[idx].ga.maxIndex);
            if (res > left + entireChild[idx].ga.maxIndex) return false;
          }

          if (entireData[res].first != key) return false;
          entireChild[idx].ga.flagNumber--;
          entireData[res] = {DBL_MIN, DBL_MIN};
          if (res == left + entireChild[idx].ga.maxIndex)
            entireChild[idx].ga.maxIndex--;
          return true;
        }
      } break;
      case 10: {
        auto size = entireChild[idx].ycsbLeaf.flagNumber & 0x00FFFFFF;
        int preIdx = entireChild[idx].ycsbLeaf.Predict(key);
        auto left = entireChild[idx].ycsbLeaf.m_left;
        if (entireData[left + preIdx].first == key) {
          entireData.erase(entireData.begin() + left + preIdx);
          return true;
        } else {
          int start = max(0, preIdx - entireChild[idx].ycsbLeaf.error) + left;
          int end =
              min(size - 1, preIdx + entireChild[idx].ycsbLeaf.error) + left;
          start = min(start, end);
          int res;
          if (key <= entireData[start].first)
            res = YCSBBinarySearch(key, left, start);
          else if (key <= entireData[end].first)
            res = YCSBBinarySearch(key, start, end);
          else {
            res = YCSBBinarySearch(key, end, left + size - 1);
            if (res >= left + size) return false;
          }
          if (entireData[res].first == key) {
            entireData.erase(entireData.begin() + res);
            return true;
          }
          return false;
        }
      } break;
    }
  }
}

#endif  // !DELETE_FUNCTION_H