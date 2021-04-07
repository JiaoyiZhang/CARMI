/**
 * @file functions.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
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

/**
 * @brief the function of using CARMI
 *
 * @param isZipfian whether to use zipfian access during the test
 * @param initRatio the workload type
 * @param rate the weight of space
 * @param thre the kAlgorithmThreshold
 * @param length the length of range scan
 * @param initDataset
 * @param testInsertQuery
 */
void CoreCARMI(bool isZipfian, double initRatio, double rate, int thre,
               const std::vector<int> &length,
               const carmi_params::TestDataVecType &initDataset,
               const carmi_params::TestDataVecType &testInsertQuery);

/**
 * @brief the function of using external CARMI
 *
 * @param isZipfian whether to use zipfian access during the test
 * @param initRatio the workload type
 * @param rate the weight of space
 * @param thre the kAlgorithmThreshold
 * @param length the length of range scan
 * @param initDataset
 * @param testInsertQuery
 */
void CoreExternalCARMI(bool isZipfian, double initRatio, double rate, int thre,
                       const std::vector<int> &length,
                       const carmi_params::TestDataVecType &initDataset,
                       const carmi_params::TestDataVecType &testInsertQuery);

void mainSynthetic(double initRatio, int thre, const std::vector<int> &length);
void mainYCSB(double initRatio, int thre, const std::vector<int> &length);
void mainMap(double initRatio, int thre, const std::vector<int> &length);
void mainExperiment(int thre);

#endif  // SRC_EXPERIMENT_FUNCTIONS_H_
