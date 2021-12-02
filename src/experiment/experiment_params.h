/**
 * @file experiment_params.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-05-19
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef EXPERIMENT_EXPERIMENT_PARAMS_H_
#define EXPERIMENT_EXPERIMENT_PARAMS_H_

#define PARAM_ZIPFIAN 0.99
#define DEBUG

#include <utility>
#include <vector>

typedef double KeyType;
typedef double ValueType;

typedef std::pair<double, double> DataType;
typedef std::vector<DataType> DataVecType;

static bool kPrimaryIndex = false;

const int kDatasetSize =
    1024.0 / sizeof(DataType) * 1024 * 1024;  // 1 GB / 16 byte
const float kTestSize = 100000.0;
const float kMaxValue = 100000000;

const float kReadOnly = 1;
const float kWriteHeavy = 0.5;
const float kReadHeavy = 0.95;
const float kWritePartial = 0.85;
const float kRangeScan = 2;

const float kSecondToNanosecond = 1000000000.0;

#ifdef DEBUG
const std::vector<double> rate = {0.025};
#else
const std::vector<double> rate = {0.01, 0.02, 0.025, 0.03, 0.05, 0.1};
#endif  // !DEBUG

#endif  // EXPERIMENT_EXPERIMENT_PARAMS_H_
