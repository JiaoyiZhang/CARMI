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

NodeCost CARMI::dp(const IndexPair &range) {
  NodeCost nodeCost;
  if (range.initRange.size == 0 && range.findRange.size == 0) {
    nodeCost = {0, 0, 0, false};
    return nodeCost;
  }

  if (range.initRange.size <= kMaxKeyNum) {
    return dpLeaf(range);
  } else if (range.initRange.size > 4096) {
    return dpInner(range);
  } else {
    auto res0 = dpLeaf(range);
    auto res1 = dpInner(range);
    if (res0.space + res0.time > res1.space + res1.time)
      return res1;
    else
      return res0;
  }
}

#endif  // SRC_CARMI_CONSTRUCT_DP_H_
