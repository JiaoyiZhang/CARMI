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
#ifndef CALCULATE_SPACE_H
#define CALCULATE_SPACE_H

#include <vector>

#include "../carmi.h"
using namespace std;

long double CARMI::calculateSpace() const {
  long double space = 0;

  switch (rootType) {
    case 0:
      space += sizeof(LRType);
      break;
    case 1:
      space += sizeof(PLRType);
      break;
    case 2:
      space += sizeof(HisType);
      break;
    case 3:
      space += sizeof(BSType);
      break;
  }

  space += 64 * nowChildNumber;
  if (!kPrimaryIndex) space += entireData.size() * 16;

  space = space / 1024 / 1024;

#ifdef DEBUG
  cout << "Space of different classes (sizeof):" << endl;
  cout << "LRType:" << sizeof(LRType)
       << "\tlr model:" << sizeof(LinearRegression) << endl;
  cout << "PLRType:" << sizeof(PLRType) << "\tnn model:" << sizeof(PiecewiseLR)
       << endl;
  cout << "HisType:" << sizeof(HisType)
       << "\this model:" << sizeof(HistogramModel) << endl;
  cout << "BSType:" << sizeof(BSType)
       << "\tbs model:" << sizeof(BinarySearchModel) << endl;
  cout << "LRModel:" << sizeof(LRModel) << endl;
  cout << "PLRModel:" << sizeof(PLRModel) << endl;
  cout << "HisModel:" << sizeof(HisModel) << endl;
  cout << "BSModel:" << sizeof(BSModel) << endl;
  cout << "ArrayType:" << sizeof(ArrayType) << endl;
  cout << "GappedArrayType:" << sizeof(GappedArrayType) << endl;
  cout << "YCSBLeaf:" << sizeof(YCSBLeaf) << endl;

  cout << "\tStructure SPACE: " << space << "MB" << endl;
  cout << "\nTOTAL SPACE (include data): " << space << "MB" << endl;
#endif  // DEBUG

  return space;
}

#endif  // !CALCULATE_SPACE_H
