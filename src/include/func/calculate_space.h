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

/**
 * @brief calculate the space of CARMI
 *
 * @return long double the space
 */
template <typename KeyType, typename ValueType>
long double CARMI<KeyType, ValueType>::CalculateSpace() const {
  long double space_cost = 0;

  switch (rootType) {
    case LR_ROOT_NODE:
      space_cost += kLRRootSpace;
      break;
    case PLR_ROOT_NODE:
      space_cost += kPLRRootSpace;
      break;
    case HIS_ROOT_NODE:
      space_cost += kHisRootSpace;
      break;
    case BS_ROOT_NODE:
      space_cost += kBSRootSpace;
      break;
  }

  space_cost += carmi_params::kBaseNodeSpace * nowChildNumber;
  if (!carmi_params::kPrimaryIndex) {
    space_cost += entireData.size() * carmi_params::kDataPointSize;
  }

#ifdef DEBUG
  std::cout << "\tStructure SPACE: " << space_cost << "MB" << std::endl;
  std::cout << "\nTOTAL SPACE (include data): " << space_cost << "MB"
            << std::endl;
#endif  // DEBUG

  return space_cost;
}

#endif  // SRC_INCLUDE_FUNC_CALCULATE_SPACE_H_
