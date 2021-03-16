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

long double CARMI::calculateSpace() const {
  long double space_cost = 0;

  switch (rootType) {
    case LR_ROOT_NODE:
      space_cost += sizeof(LRType) / 1024 / 1024;
      break;
    case PLR_ROOT_NODE:
      space_cost += sizeof(PLRType) / 1024 / 1024;
      break;
    case HIS_ROOT_NODE:
      space_cost += sizeof(HisType) / 1024 / 1024;
      break;
    case BS_ROOT_NODE:
      space_cost += sizeof(BSType) / 1024 / 1024;
      break;
  }

  space_cost += kBaseNodeSpace * nowChildNumber;
  if (!kPrimaryIndex) {
    space_cost += entireData.size() * kDataPointSize;
  }

#ifdef DEBUG
  std::cout << "Space of different classes (sizeof):" << std::endl;
  std::cout << "LRType:" << sizeof(LRType)
            << "\tlr model:" << sizeof(LinearRegression) << std::endl;
  std::cout << "PLRType:" << sizeof(PLRType)
            << "\tnn model:" << sizeof(PiecewiseLR) << std::endl;
  std::cout << "HisType:" << sizeof(HisType)
            << "\this model:" << sizeof(HistogramModel) << std::endl;
  std::cout << "BSType:" << sizeof(BSType)
            << "\tbs model:" << sizeof(BinarySearchModel) << std::endl;
  std::cout << "LRModel:" << sizeof(LRModel) << std::endl;
  std::cout << "PLRModel:" << sizeof(PLRModel) << std::endl;
  std::cout << "HisModel:" << sizeof(HisModel) << std::endl;
  std::cout << "BSModel:" << sizeof(BSModel) << std::endl;
  std::cout << "ArrayType:" << sizeof(ArrayType) << std::endl;
  std::cout << "GappedArrayType:" << sizeof(GappedArrayType) << std::endl;
  std::cout << "YCSBLeaf:" << sizeof(YCSBLeaf) << std::endl;

  std::cout << "\tStructure SPACE: " << space_cost << "MB" << std::endl;
  std::cout << "\nTOTAL SPACE (include data): " << space_cost << "MB"
            << std::endl;
#endif  // DEBUG

  return space_cost;
}

#endif  // SRC_CARMI_FUNC_CALCULATE_SPACE_H_
