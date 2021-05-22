/**
 * @file params.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_PARAMS_H_
#define SRC_INCLUDE_PARAMS_H_

#define DEBUG

namespace carmi_params {

const int kAlgorithmThreshold = 1024;
const int kLeafMaxCapacity = 4096;  // the max capacity of a leaf node

const double kLRRootTime = 12.7013;  // the time cost of lr root

const double kLRInnerTime = 92.4801;    // the time cost of lr inner node
const double kPLRInnerTime = 97.1858;   // the time cost of plr inner node
const double kHisInnerTime = 109.8874;  // the time cost of his inner node
const double kBSInnerTime = 114.371;    // the time cost of bs inner node

// TODO(jiaoyi): modify the value due to the change of leaf nodes
const double kCostMoveTime = 6.25;     // the time cost of moving a data point
const double kLeafBaseTime = 161.241;  // the basic time cost of a leaf node
const double kCostBSTime =
    10.9438;  // the time cost of binary search in a leaf node

}  // namespace carmi_params

#endif  // SRC_INCLUDE_PARAMS_H_
