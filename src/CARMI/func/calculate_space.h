/**
 * @file calculate_space.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_FUNC_CALCULATE_SPACE_H_
#define SRC_CARMI_FUNC_CALCULATE_SPACE_H_

#include <vector>

#include "../carmi.h"

long double CARMI::CalculateSpace() const {
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

  space_cost += kBaseNodeSpace * nowChildNumber;
  if (!kPrimaryIndex) {
    space_cost += entireData.size() * kDataPointSize;
  }

#ifdef DEBUG
  std::cout << "\tStructure SPACE: " << space_cost << "MB" << std::endl;
  std::cout << "\nTOTAL SPACE (include data): " << space_cost << "MB"
            << std::endl;
#endif  // DEBUG

  return space_cost;
}

#endif  // SRC_CARMI_FUNC_CALCULATE_SPACE_H_
