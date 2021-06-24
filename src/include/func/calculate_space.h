/**
 * @file calculate_space.h
 * @author Jiaoyi
 * @brief calculate the space of CARMI
 * @version 0.1
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

  switch (rootType) {
    case LR_ROOT_NODE:
      space_cost += kLRRootSpace;
      break;
  }

  space_cost += kBaseNodeSpace * nowChildNumber;
  std::cout << "entireChild.size(): " << entireChild.size()
            << ",\tnowChildNumber:" << nowChildNumber << std::endl;
  std::cout << "entireData.size(): " << entireData.size()
            << ",\tkMaxLeafNodeSize:" << carmi_params::kMaxLeafNodeSize
            << std::endl;
  if (!isPrimary) {
    space_cost += entireData.size() * carmi_params::kMaxLeafNodeSize;
  }
  return space_cost / 1024 / 1024;
}

#endif  // SRC_INCLUDE_FUNC_CALCULATE_SPACE_H_
