/**
 * @file public_functions.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-04-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_WORKLOAD_PUBLIC_FUNCTIONS_H_
#define SRC_EXPERIMENT_WORKLOAD_PUBLIC_FUNCTIONS_H_

#include <chrono>
#include <utility>
#include <vector>

#include "../../include/carmi_common.h"
#include "../../include/func/find_function.h"
#include "../../include/func/insert_function.h"
#include "./zipfian.h"

extern std::ofstream outRes;

/**
 * @brief prepare query workloads
 *
 * @param Ratio the ratio of find queries
 * @param findQueryset
 * @param insertDataset
 * @param findQuery
 * @param insertQuery
 * @param index
 */
void InitTestSet(double Ratio,
                 const carmi_params::TestDataVecType &findQueryset,
                 const carmi_params::TestDataVecType &insertDataset,
                 carmi_params::TestDataVecType *findQuery,
                 carmi_params::TestDataVecType *insertQuery,
                 std::vector<int> *index);

/**
 * @brief print the average time of the workload
 *
 * @param time
 */
void PrintAvgTime(double time);

#endif  // SRC_EXPERIMENT_WORKLOAD_PUBLIC_FUNCTIONS_H_