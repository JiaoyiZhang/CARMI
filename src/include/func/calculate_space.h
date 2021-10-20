/**
 * @file calculate_space.h
 * @author Jiaoyi
 * @brief calculate the space of CARMI
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_FUNC_CALCULATE_SPACE_H_
#define SRC_INCLUDE_FUNC_CALCULATE_SPACE_H_

#include <vector>

#include "../carmi.h"
#include "../params.h"

template <typename KeyType, typename ValueType>
long double CARMI<KeyType, ValueType>::CalculateSpace() const {
  long double space_cost = 0;

  switch (root.flagNumber) {
    case PLR_ROOT_NODE:
      space_cost += kPLRRootSpace;
      break;
  }

  space_cost += kBaseNodeSpace * node.nowNodeNumber;
  std::cout << "node.size(): " << node.nodeArray.size()
            << ",\tnowChildNumber:" << node.nowNodeNumber << std::endl;
  std::cout << "data.size(): " << data.dataArray.size()
            << ",\tkMaxLeafNodeSize:" << carmi_params::kMaxLeafNodeSize
            << std::endl;
  if (!isPrimary) {
    space_cost +=
        data.dataArray.size() * carmi_params::kMaxLeafNodeSize / 1024 / 1024;
  }
  return space_cost;
}

#endif  // SRC_INCLUDE_FUNC_CALCULATE_SPACE_H_
