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
#ifndef FUNC_CALCULATE_SPACE_H_
#define FUNC_CALCULATE_SPACE_H_

#include <vector>

#include "../carmi.h"
#include "../params.h"

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
long long CARMI<KeyType, ValueType, Compare, Alloc>::CalculateSpace() const {
  // calculate the space of the plr root node
  long long space_cost = kPLRRootSpace * 1024.0 * 1024.0;
  // calculate the space of the node array
  space_cost += kBaseNodeSpace * node.nowNodeNumber * 1024.0 * 1024.0;
#ifdef DEBUG
  std::cout << "node.size(), " << node.nodeArray.size() << ",\tnowChildNumber,"
            << node.nowNodeNumber << std::endl;
  std::cout << "data.size(), " << data.dataArray.size()
            << ",\tkMaxLeafNodeSize," << carmi_params::kMaxLeafNodeSize
            << std::endl;
#endif  // DEBUG

  if (!isPrimary) {
    // calculate the space of the data array
    space_cost += static_cast<double>(data.dataArray.size()) *
                  carmi_params::kMaxLeafNodeSize;
  }
  return space_cost;
}

#endif  // FUNC_CALCULATE_SPACE_H_
