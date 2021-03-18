/**
 * @file dp.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_CONSTRUCT_DP_H_
#define SRC_CARMI_CONSTRUCT_DP_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <vector>

#include "../../params.h"
#include "../func/inlineFunction.h"
#include "./dp_inner.h"
#include "./dp_leaf.h"
#include "./greedy.h"
#include "./structures.h"
/**
 * @brief the dynamic programming algorithm
 *
 * @param range the range of data points
 * @return NodeCost the cost of the subtree
 */
NodeCost CARMI::DP(const DataRange &range) {
  NodeCost nodeCost;
  if (range.initRange.size == 0) {
    nodeCost = emptyCost;
    ConstructEmptyNode(range);
    return nodeCost;
  }

  auto it = COST.find(range.initRange);
  if (it != COST.end()) {
    nodeCost = it->second;
    return nodeCost;
  }

  if (range.initRange.size <= kMaxKeyNum) {
    return DPLeaf(range);
  } else if (range.initRange.size > kLeafMaxCapacity) {
    return DPInner(range);
  } else {
    auto res1 = DPInner(range);
    auto res0 = DPLeaf(range);
    if (res0.space * kRate + res0.time > res1.space * kRate + res1.time)
      return res1;
    else
      return res0;
  }
}

#endif  // SRC_CARMI_CONSTRUCT_DP_H_
