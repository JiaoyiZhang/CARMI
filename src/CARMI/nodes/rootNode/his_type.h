/**
 * @file his_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_ROOTNODE_HIS_TYPE_H_
#define SRC_CARMI_NODES_ROOTNODE_HIS_TYPE_H_

#include <vector>

#include "../../../params.h"
#include "trainModel/histogram.h"

class HisType {
 public:
  HisType() = default;
  explicit HisType(int c) {
    flagNumber = (2 << 24) + c;
    model = HistogramModel(c);
  }

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;         // 4 Byte
  HistogramModel model;  // 16+16c
};

#endif  // SRC_CARMI_NODES_ROOTNODE_HIS_TYPE_H_
