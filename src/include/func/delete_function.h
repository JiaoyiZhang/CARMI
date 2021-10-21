/**
 * @file delete_function.h
 * @author Jiaoyi
 * @brief delete a record
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef FUNC_DELETE_FUNCTION_H_
#define FUNC_DELETE_FUNCTION_H_

#include <algorithm>

#include "../carmi.h"
#include "inlineFunction.h"

template <typename KeyType, typename ValueType>
bool CARMI<KeyType, ValueType>::Delete(const KeyType &key) {
  int idx = 0;  // idx in the INDEX
  int type = root.flagNumber;
  while (1) {
    switch (type) {
      case PLR_ROOT_NODE:
        idx = root.PLRType<DataVectorType, KeyType>::model.Predict(key);
        break;
      case LR_INNER_NODE:
        idx = node.nodeArray[idx].lr.childLeft +
              node.nodeArray[idx].lr.Predict(key);
        break;
      case PLR_INNER_NODE:
        idx = node.nodeArray[idx].plr.childLeft +
              node.nodeArray[idx].plr.Predict(key);
        break;
      case HIS_INNER_NODE:
        idx = node.nodeArray[idx].his.childLeft +
              node.nodeArray[idx].his.Predict(key);
        break;
      case BS_INNER_NODE:
        idx = node.nodeArray[idx].bs.childLeft +
              node.nodeArray[idx].bs.Predict(key);
        break;
      case ARRAY_LEAF_NODE: {
        return node.nodeArray[idx].cfArray.Delete(key, &data);
      }
    }

    type = node.nodeArray[idx].lr.flagNumber >> 24;
  }
}

#endif  // FUNC_DELETE_FUNCTION_H_
