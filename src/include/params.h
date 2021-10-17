/**
 * @file params.h
 * @author Jiaoyi
 * @brief parameters in carmi_params space
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_PARAMS_H_
#define SRC_INCLUDE_PARAMS_H_

#define DEBUG
// #define Windows

#ifndef Windows
#define Ubuntu
#endif  // Windows

namespace carmi_params {
const int kMaxLeafNodeSize = 256;  // bytes, the size of a union in leaf nodes
const int kMaxLeafNodeSizeExternal = 1024;

const int kAlgorithmThreshold = 1024;
const float kRootPrefetchRatio = 0.4;

const double kMemoryAccessTime = 80.09;  // the time cost of memory access

const double kLRRootTime = 12.7013;  // the time cost of lr root
const double kPLRRootTime = 29.62;   // the time cost of plr root

const double kLRInnerTime =
    kMemoryAccessTime + 14.2;  // the time cost of lr inner node
const double kPLRInnerTime =
    kMemoryAccessTime + 22.8;  // the time cost of plr inner node
const double kHisInnerTime =
    kMemoryAccessTime + 9.8;  // the time cost of his inner node
const double kBSInnerTime =
    kMemoryAccessTime + 15.5;  // the time cost of bs inner node

const double kCostMoveTime = 6.25;     // the time cost of moving a data point
const double kLeafBaseTime = 161.241;  // the basic time cost of a leaf node
const double kCostBSTime =
    10.9438;  // the time cost of binary search in a leaf node
}  // namespace carmi_params

#endif  // SRC_INCLUDE_PARAMS_H_
