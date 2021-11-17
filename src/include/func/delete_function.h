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

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
bool CARMI<KeyType, ValueType, Compare, Alloc>::Delete(const KeyType &key,
                                                       size_t *cnt) {
  int idx = 0;  // idx in the node array
  int type = root.flagNumber;
  while (1) {
    switch (type) {
      case PLR_ROOT_NODE:
        // Case 0: this node is the plr root node
        // use the plr root node to find the index of the next node
        idx = root.PLRType<DataVectorType, KeyType>::model.Predict(key);
        break;
      case LR_INNER_NODE:
        // Case 1: this node is the lr inner node
        // use the predict function of lr inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].lr.Predict(key);
        break;
      case PLR_INNER_NODE:
        // Case 2: this node is the plr inner node
        // use the predict function of plr inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].plr.Predict(key);
        break;
      case HIS_INNER_NODE:
        // Case 3: this node is the his inner node
        // use the predict function of his inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].his.Predict(key);
        break;
      case BS_INNER_NODE:
        // Case 4: this node is the bs inner node
        // use the predict function of bs inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].bs.Predict(key);
        break;
      case ARRAY_LEAF_NODE: {
        // Case 5: this node is the cache-friendly array leaf node
        // Delete the data point in the cf leaf node
        return node.nodeArray[idx].cfArray.Delete(key, cnt, &data);
      }
    }

    type = node.nodeArray[idx].lr.flagNumber >> 24;
  }
}

#endif  // FUNC_DELETE_FUNCTION_H_
