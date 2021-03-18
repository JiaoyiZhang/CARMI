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
#ifndef SRC_PARAMS_H_
#define SRC_PARAMS_H_

#define ZIPFIAN 1
#define PARAM_ZIPFIAN 0.99
// #define DEBUG

#include <utility>
#include <vector>

typedef std::pair<double, double> DataType;
typedef std::vector<DataType> DataVectorType;

// const bool kPrimaryIndex = false;
bool kPrimaryIndex = false;

const double kDensity = 0.5;         // the density of gap in gapped array
const double kExpansionScale = 1.5;  // scale of array expansion

const int kLeafMaxCapacity = 4096;
const int kAlgorithmThre = 4096;
const int kReservedSpace = 1024 * 512;

const double kLRRootTime = 12.7013;
const double kPLRRootTime = 39.6429;
const double kHisRootTime = 44.2824;

const double kLRInnerTime = 92.4801;
const double kPLRInnerTime = 97.1858;
const double kHisInnerTime = 109.8874;
const double kBSInnerTime = 114.371;

const double kCostMoveTime = 6.25;
const double kLeafBaseTime = 161.241;
const double kCostBSTime = 10.9438;

const int kHisMaxChildNumber = 256;
const int kBSMaxChildNumber = 20;
const int kMinChildNumber = 16;
const int kInsertNewChildNumber = 128;

const double kBaseNodeSpace = 64.0 / 1024 / 1024;  // MB
const float kDataPointSize = 16.0 / 1024 / 1024;   // MB
const int kDatasetSize = 1024.0 / kDataPointSize;  // 1 GB / 16 bytes
const float kTestSize = 100000.0;
const float kMaxValue = 100000000;

const float kReadOnly = 1;
const float kWriteHeavy = 0.5;
const float kReadHeavy = 0.95;
const float kWritePartial = 0.85;
const float kRangeScan = 2;

const float kSecondToNanosecond = 1000000000.0;

const float Density[5] = {0.5, 0.6, 0.7, 0.8, 0.9};  // size / capacity

const int kRMIInnerChild = 32;

#endif  // SRC_PARAMS_H_
