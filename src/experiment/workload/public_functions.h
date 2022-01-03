/**
 * @file public_functions.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-04-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef EXPERIMENT_WORKLOAD_PUBLIC_FUNCTIONS_H_
#define EXPERIMENT_WORKLOAD_PUBLIC_FUNCTIONS_H_

#include <utility>
#include <vector>

#include "../../include/carmi_map.h"
#include "../experiment_params.h"
#include "./zipfian.h"

extern std::ofstream outRes;

/**
 * @brief prepare query workloads
 *
 * @param[in] findQueryset
 * @param[in] insertDataset
 * @param[inout] findQuery
 * @param[inout] insertQuery
 * @param[inout] index
 */
void InitTestSet(const DataVecType &findQueryset,
                 const DataVecType &insertDataset, bool isZipfian,
                 DataVecType *findQuery, DataVecType *insertQuery,
                 std::vector<int> *index);

/**
 * @brief print the average time of the workload
 *
 * @param[in] time
 */
void PrintAvgTime(double time);

#endif  // EXPERIMENT_WORKLOAD_PUBLIC_FUNCTIONS_H_
