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
/**
 * @brief bytes, the size of a data block in cf array leaf nodes
 */
static constexpr int kMaxLeafNodeSize = 256;

/**
 * @brief the maximum number of data points in an external leaf node
 */
static constexpr int kMaxLeafNodeSizeExternal = 1024;

/**
 * @brief the maximum number of data points which can use the DP algorithm to
 * construct an inner node. If the size of the sub-dataset exceeds this
 * parameter, greedy algorithm is used to construct the node.
 */
static constexpr int kAlgorithmThreshold = 1024;

/**
 * @brief the latency of memory access
 */
static constexpr double kMemoryAccessTime = 80.09;

/**
 * @brief the CPU time cost of lr root
 */
static constexpr double kLRRootTime = 12.7013;

/**
 * @brief the CPU time cost of plr root
 */
static constexpr double kPLRRootTime = 29.62;

/**
 * @brief the time cost of lr inner node
 */
static constexpr double kLRInnerTime = kMemoryAccessTime + 14.2;

/**
 * @brief the time cost of plr inner node
 */
static constexpr double kPLRInnerTime = kMemoryAccessTime + 22.8;

/**
 * @brief the time cost of his inner node
 */
static constexpr double kHisInnerTime = kMemoryAccessTime + 9.8;

/**
 * @brief the time cost of bs inner node
 */
static constexpr double kBSInnerTime = kMemoryAccessTime + 15.5;

/**
 * @brief the time cost of moving a data point
 */
static constexpr double kCostMoveTime = 6.25;

/**
 * @brief the basic time cost of a leaf node
 */
static constexpr double kLeafBaseTime = 161.241;

/**
 * @brief the time cost of a binary search
 */
static constexpr double kCostBSTime = 10.9438;
}  // namespace carmi_params

#endif  // PARAMS_H_
