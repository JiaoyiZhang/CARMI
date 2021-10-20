/**
 * @file update_function.h
 * @author Jiaoyi
 * @brief update a record
 * @version 3.0
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
bool CARMI<KeyType, ValueType>::Update(const DataType &datapoint) {
  int idx = 0;  // idx in the INDEX
  int type = root.flagNumber;
  while (1) {
    switch (type) {
      case PLR_ROOT_NODE:
        idx = root.PLRType<DataVectorType, KeyType>::model.Predict(
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
        return node.nodeArray[idx].cfArray.Update(datapoint, &data);
      }
    }

    type = node.nodeArray[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_INCLUDE_FUNC_UPDATE_FUNCTION_H_
