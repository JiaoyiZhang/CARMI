/**
 * @file insert_function.h
 * @author Jiaoyi
 * @brief insert a record
 * @version 3.0
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
bool CARMI<KeyType, ValueType>::Insert(const DataType &datapoint) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  while (1) {
    switch (type) {
      case PLR_ROOT_NODE:
        idx = root.childLeft +
              root.PLRType<DataVectorType, KeyType>::model.Predict(
                  datapoint.first);
        break;
      case LR_INNER_NODE:
        idx = node.nodeArray[idx].lr.childLeft +
              node.nodeArray[idx].lr.Predict(datapoint.first);
        break;
      case PLR_INNER_NODE:
        idx = node.nodeArray[idx].plr.childLeft +
              node.nodeArray[idx].plr.Predict(datapoint.first);
        break;
      case HIS_INNER_NODE:
        idx = node.nodeArray[idx].his.childLeft +
              node.nodeArray[idx].his.Predict(datapoint.first);
        break;
      case BS_INNER_NODE:
        idx = node.nodeArray[idx].bs.childLeft +
              node.nodeArray[idx].bs.Predict(datapoint.first);
        break;
      case ARRAY_LEAF_NODE: {
        bool isSuccess = node.nodeArray[idx].cfArray.Insert(datapoint, &data);
        if (isSuccess) {
          return true;
        } else {
          // split
          int previousIdx = node.nodeArray[idx].cfArray.previousLeaf;
          Split<CFArrayType<KeyType, ValueType>>(
              false, node.nodeArray[idx].cfArray.m_left,
              node.nodeArray[idx].cfArray.kMaxBlockNum, previousIdx, idx);
          idx = node.nodeArray[idx].lr.childLeft +
                node.nodeArray[idx].lr.Predict(datapoint.first);
        }
        break;
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        bool isSuccess =
            node.nodeArray[idx].externalArray.Insert(datapoint, &curr);
        if (isSuccess) {
          return true;
        } else {
          // split
          int size = node.nodeArray[idx].externalArray.flagNumber & 0x00FFFFFF;
          Split<ExternalArray<KeyType>>(
              true, node.nodeArray[idx].externalArray.m_left, size, 0, idx);
          idx = node.nodeArray[idx].lr.childLeft +
                node.nodeArray[idx].lr.Predict(datapoint.first);
        }
      }
    }

    type = node.nodeArray[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_INCLUDE_FUNC_INSERT_FUNCTION_H_
