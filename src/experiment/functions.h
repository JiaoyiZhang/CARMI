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
#ifndef SRC_EXPERIMENT_FUNCTIONS_H_
#define SRC_EXPERIMENT_FUNCTIONS_H_

#include <vector>

#include "../include/carmi_common.h"
#include "../include/carmi_external.h"
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
 * @param Ratio the ratio of find queries
 * @param findQueryset
 * @param insertDataset
 * @param findQuery
 * @param insertQuery
 * @param index
 */
void InitTestSet(double Ratio, const DataVecType &findQueryset,
                 const DataVecType &insertDataset, DataVecType *findQuery,
                 DataVecType *insertQuery, std::vector<int> *index);

/**
 * @brief print the average time of the workload
 *
 * @param time
 */
void PrintAvgTime(double time);

/**
 * @brief the function of using CARMI
 *
 * @param isZipfian whether to use zipfian access during the test
 * @param initRatio the workload type
 * @param rate the weight of space
 * @param length the length of range scan
 * @param initDataset
 * @param testInsertQuery
 */
void CoreCARMI(bool isZipfian, double initRatio, double rate,
               const std::vector<int> &length, const DataVecType &initDataset,
               const DataVecType &testInsertQuery);

/**
 * @brief the function of using external CARMI
 *
 * @param isZipfian whether to use zipfian access during the test
 * @param initRatio the workload type
 * @param rate the weight of space
 * @param length the length of range scan
 * @param initDataset
 * @param testInsertQuery
 */
void CoreExternalCARMI(bool isZipfian, double initRatio, double rate,
                       const std::vector<int> &length,
                       const DataVecType &initDataset,
                       const DataVecType &testInsertQuery);

void mainSynthetic(double initRatio, const std::vector<int> &length);
void mainYCSB(double initRatio, const std::vector<int> &length);
void mainMap(double initRatio, const std::vector<int> &length);
void mainExperiment();

#endif  // SRC_EXPERIMENT_FUNCTIONS_H_
