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
#ifndef PARAMS_H_
#define PARAMS_H_

#define DEBUG
// #define Windows

#ifndef Windows
#define Ubuntu
#endif  // Windows

namespace carmi_params {
// bytes, the size of a data block in cf array leaf nodes
const int kMaxLeafNodeSize = 256;

// the maximum number of data points in an external leaf node
const int kMaxLeafNodeSizeExternal = 1024;

/**
 * @brief the maximum number of data points which can use the DP algorithm to
 * construct an inner node. If the size of the sub-dataset exceeds this
 * parameter, greedy algorithm is used to construct the node.
 */
const int kAlgorithmThreshold = 1024;

// the latency of memory access
const double kMemoryAccessTime = 80.09;

// the CPU time cost of lr root
const double kLRRootTime = 12.7013;
// the CPU time cost of plr root
const double kPLRRootTime = 29.62;

// the time cost of lr inner node
const double kLRInnerTime = kMemoryAccessTime + 14.2;
// the time cost of plr inner node
const double kPLRInnerTime = kMemoryAccessTime + 22.8;
// the time cost of his inner node
const double kHisInnerTime = kMemoryAccessTime + 9.8;
// the time cost of bs inner node
const double kBSInnerTime = kMemoryAccessTime + 15.5;

// the time cost of moving a data point
const double kCostMoveTime = 6.25;
// the basic time cost of a leaf node
const double kLeafBaseTime = 161.241;
// the time cost of a binary search
const double kCostBSTime = 10.9438;
}  // namespace carmi_params

#endif  // PARAMS_H_
