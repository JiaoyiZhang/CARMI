/**
 * @file insert_function.h
 * @author Jiaoyi
 * @brief insert a record
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_FUNC_INSERT_FUNCTION_H_
#define SRC_INCLUDE_FUNC_INSERT_FUNCTION_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "../construct/minor_function.h"

template <typename KeyType, typename ValueType>
bool CARMI<KeyType, ValueType>::Insert(DataType data) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  int childIdx = 0;
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
        CheckChildBound(idx);
        int nowLeafNum = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        int left = entireChild[idx].array.m_left;
        int currunion = entireChild[idx].array.Predict(data.first);

        if (nowLeafNum == 0) {
          bool isSuccess =
              SlotsUnionInsert(data, currunion, &entireData[left + currunion],
                               &entireChild[idx]);
          if (isSuccess) {
            CheckChildBound(idx);
            return true;
          }
        } else if (currunion >= nowLeafNum) {
          bool isSuccess = SlotsUnionInsert(data, currunion,
                                            &entireData[left + currunion - 1],
                                            &entireChild[idx]);
          if (isSuccess) {
            CheckChildBound(idx);
            entireChild[idx].array.slotkeys[currunion - 1] = std::max(
                entireChild[idx].array.slotkeys[currunion - 1], data.first + 1);
            return true;
          } else {
            // expand
            Expand(left, left + nowLeafNum, &entireChild[idx].array);
            break;
          }
        }
        bool isSuccess = SlotsUnionInsert(
            data, currunion, &entireData[left + currunion], &entireChild[idx]);
        if (isSuccess) {
          CheckChildBound(idx);
          return true;
        }
        int nowDataNum = GetDataNum(left, left + nowLeafNum);

        left += currunion;
        // insert into the sibling
        if (currunion == 0 && nowLeafNum != 1) {
#ifdef DEBUG
          CheckBound(left, 1, nowDataSize);
#endif  // DEBUG
        // only check the next union
          if (entireData[left + 1].slots[kMaxSlotNum - 1].first == DBL_MIN) {
            CheckChildBound(idx);
            isSuccess =
                ArrayInsertNext(data, left, currunion, &entireChild[idx]);
            CheckChildBound(idx);
            if (isSuccess) {
              return true;
            }
          }
        } else if (currunion == nowLeafNum - 1 && currunion != 0) {
#ifdef DEBUG
          CheckBound(left, -1, nowDataSize);
#endif  // DEBUG
#ifdef DEBUG
          CheckBound(left, 0, nowDataSize);
#endif  // DEBUG
        // only check the previous union
          if (entireData[left - 1].slots[kMaxSlotNum - 1].first == DBL_MIN &&
              (entireData[left].slots[0].first !=
               entireData[left].slots[1].first)) {
            isSuccess =
                ArrayInsertPrevious(data, left, currunion, &entireChild[idx]);
            if (isSuccess) {
              CheckChildBound(idx);
              return true;
            }
          }
        } else if (currunion > 0 && currunion < nowLeafNum - 1) {
#ifdef DEBUG
          CheckBound(left, -1, nowDataSize);
#endif  // DEBUG
#ifdef DEBUG
          CheckBound(left, 0, nowDataSize);
#endif  // DEBUG
          if (entireData[left - 1].slots[kMaxSlotNum - 1].first == DBL_MIN &&
              (entireData[left].slots[0].first !=
               entireData[left].slots[1].first)) {
            isSuccess =
                ArrayInsertPrevious(data, left, currunion, &entireChild[idx]);
            if (isSuccess) {
              CheckChildBound(idx);
              return true;
            }
          }
#ifdef DEBUG
          CheckBound(left, 1, nowDataSize);
#endif  // DEBUG
          if (entireData[left + 1].slots[kMaxSlotNum - 1].first == DBL_MIN) {
            isSuccess =
                ArrayInsertNext(data, left, currunion, &entireChild[idx]);
            if (isSuccess) {
              CheckChildBound(idx);
              return true;
            }
          }
        }
        if (nowDataNum < nowLeafNum * (kMaxSlotNum - 1)) {
          // rebalance
          Rebalance(left - currunion, left - currunion + nowLeafNum,
                    &entireChild[idx].array);
        } else if (nowLeafNum >= kMaxLeafNum) {
          // split
          int previousIdx = entireChild[idx].array.previousLeaf;
          Split<ArrayType<KeyType>>(false, left - currunion, kMaxLeafNum,
                                    previousIdx, idx);
          idx = entireChild[idx].lr.childLeft +
                entireChild[idx].lr.Predict(data.first);

          CheckChildBound(idx);
        } else {
          // expand
          Expand(left - currunion, left - currunion + nowLeafNum,
                 &entireChild[idx].array);
        }
        break;
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        int left = entireChild[idx].externalArray.m_left;
        int size = entireChild[idx].externalArray.flagNumber & 0x00FFFFFF;

        // split
        if (size >= kLeafMaxCapacity) {
          Split<ExternalArray>(true, left, size, 0, idx);
          idx = entireChild[idx].lr.childLeft +
                entireChild[idx].lr.Predict(data.first);
          left = entireChild[idx].externalArray.m_left;
          size = entireChild[idx].externalArray.flagNumber & 0x00FFFFFF;
        }
        if (size > 0) {
          entireChild[idx].externalArray.flagNumber++;
        } else if (size == 0) {
          entireChild[idx].externalArray.m_left = curr;
          DataVectorType trainData;
          trainData.push_back(data);
          Train(0, 1, trainData, &entireChild[idx].externalArray);
        }

        entireChild[idx].externalArray.flagNumber++;
        curr++;

        return true;
      }
    }

    type = entireChild[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_INCLUDE_FUNC_INSERT_FUNCTION_H_
