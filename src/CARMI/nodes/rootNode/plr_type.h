/**
 * @file plr_type.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_NODES_ROOTNODE_PLR_TYPE_H_
#define SRC_CARMI_NODES_ROOTNODE_PLR_TYPE_H_

#include <vector>

#include "../../../params.h"
#include "../../construct/structures.h"
#include "trainModel/piecewiseLR.h"

class PLRType {
 public:
  PLRType() = default;
  explicit PLRType(int c) {
    flagNumber = (PLR_ROOT_NODE << 24) + c;
    model = new PiecewiseLR();
  }
  ~PLRType() { delete model; }

  int flagNumber;  // 4 Byte (flag + childNumber)

  int childLeft;       // 4 Byte
  PiecewiseLR *model;  // 24*8+4 Byte
};

#endif  // SRC_CARMI_NODES_ROOTNODE_PLR_TYPE_H_
