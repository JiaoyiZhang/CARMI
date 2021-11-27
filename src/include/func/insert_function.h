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
#ifndef FUNC_INSERT_FUNCTION_H_
#define FUNC_INSERT_FUNCTION_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "../construct/minor_function.h"
#include "./split_function.h"

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
std::pair<BaseNode<KeyType, ValueType, Compare, Alloc> *, bool>
CARMI<KeyType, ValueType, Compare, Alloc>::Insert(const DataType &datapoint,
                                                  int *currblock,
                                                  int *currslot) {
  int idx = 0;  // idx in the node array
  int type = root.flagNumber;
  while (1) {
    switch (type) {
      case PLR_ROOT_NODE:
        // Case 0: this node is the plr root node
        // use the plr root node to find the index of the next node
        idx = root.PLRType<DataVectorType, KeyType>::model.Predict(
            datapoint.first);
        break;
      case LR_INNER_NODE:
        // Case 1: this node is the lr inner node
        // use the predict function of lr inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].lr.Predict(datapoint.first);
        break;
      case PLR_INNER_NODE:
        // Case 2: this node is the plr inner node
        // use the predict function of plr inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].plr.Predict(datapoint.first);
        break;
      case HIS_INNER_NODE:
        // Case 3: this node is the his inner node
        // use the predict function of his inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].his.Predict(datapoint.first);
        break;
      case BS_INNER_NODE:
        // Case 4: this node is the bs inner node
        // use the predict function of bs inner node to obtain the index of the
        // next node
        idx = node.nodeArray[idx].bs.Predict(datapoint.first);
        break;
      case ARRAY_LEAF_NODE: {
        // Case 5: this node is the cache-friendly array leaf node
        // insert the data point in the cf leaf node
        bool isSuccess = node.nodeArray[idx].cfArray.Insert(
            datapoint, currblock, currslot, &data);
        if (isSuccess) {
          if (datapoint.first > lastKey) {
            lastLeaf = idx;
            lastKey = datapoint.first;
          }
          if (datapoint.first < firstKey) {
            firstLeaf = idx;
            firstKey = datapoint.first;
          }
          currsize++;
          return {&node.nodeArray[idx], true};
        } else {
          // if this leaf node cannot accomodate more data points, we need to
          // split it and replace it with a new inner node and several new leaf
          // nodes
          Split<CFArrayType<KeyType, ValueType, Compare, Alloc>>(idx);
          idx = node.nodeArray[idx].lr.Predict(datapoint.first);
        }
        break;
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        // Case 6: this node is the external array leaf node
        // insert the key value of the data point in the external leaf node
        bool isSuccess =
            node.nodeArray[idx].externalArray.Insert(datapoint, &currsize);

        if (isSuccess) {
          *currslot = currsize - 1;
          return {&node.nodeArray[idx], true};
        } else {
          // if this leaf node cannot accomodate more data points, we need to
          // split it and replace it with a new inner node and several new leaf
          // nodes
          Split<ExternalArray<KeyType, ValueType, Compare>>(idx);
          idx = node.nodeArray[idx].lr.Predict(datapoint.first);
        }
      }
    }

    type = node.nodeArray[idx].lr.flagNumber >> 24;
  }
}

#endif  // FUNC_INSERT_FUNCTION_H_
