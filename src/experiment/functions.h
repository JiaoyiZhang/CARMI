/**
 * @file functions.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-04-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef EXPERIMENT_FUNCTIONS_H_
#define EXPERIMENT_FUNCTIONS_H_

#include <vector>

#include "../include/carmi_external_map.h"
#include "../include/carmi_map.h"
#include "./workload/workloads.h"
#include "./workload/workloads_external.h"
#include "dataset/exponential_distribution.h"
#include "dataset/lognormal_distribution.h"
#include "dataset/longitudes.h"
#include "dataset/longlat.h"
#include "dataset/normal_distribution.h"
#include "dataset/uniform_distribution.h"
#include "dataset/ycsb.h"

/**
 * @brief prepare query workloads
 *
 * @param[in] Ratio the ratio of find queries
 * @param[in] findQueryset
 * @param[in] insertDataset
 * @param[inout] findQuery
 * @param[inout] insertQuery
 * @param[inout] index
 */
void InitTestSet(double Ratio, const DataVecType &findQueryset,
                 const DataVecType &insertDataset, DataVecType *findQuery,
                 DataVecType *insertQuery, std::vector<int> *index);

/**
 * @brief print the average time of the workload
 *
 * @param[in] time
 */
void PrintAvgTime(double time);

/**
 * @brief the function of using CARMI
 *
 * @param[in] isZipfian whether to use zipfian access during the test
 * @param[in] initRatio the workload type
 * @param[in] rate the weight of space
 * @param[in] length the length of range scan
 * @param[in] initDataset
 * @param[in] insertDataset
 * @param[in] testInsertQuery
 */
void CoreCARMI(bool isZipfian, double initRatio, double rate,
               const std::vector<int> &length, const DataVecType &initDataset,
               const DataVecType &insertDataset,
               const DataVecType &testInsertQuery);

/**
 * @brief the function of using external CARMI
 *
 * @param[in] isZipfian whether to use zipfian access during the test
 * @param[in] initRatio the workload type
 * @param[in] rate the weight of space
 * @param[in] length the length of range scan
 * @param[in] initDataset
 * @param[in] testInsertQuery
 */
void CoreExternalCARMI(bool isZipfian, double initRatio, double rate,
                       const std::vector<int> &length,
                       const DataVecType &initDataset,
                       const DataVecType &testInsertQuery);

void mainSynthetic(double initRatio, const std::vector<int> &length);
void mainYCSB(double initRatio, const std::vector<int> &length);
void mainMap(double initRatio, const std::vector<int> &length);
void mainExperiment();

#endif  // EXPERIMENT_FUNCTIONS_H_
