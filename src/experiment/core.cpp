/**
 * @file core.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_CORE_H_
#define SRC_EXPERIMENT_CORE_H_
#include <vector>

#include "../include/carmi_common.h"
#include "../include/carmi_external.h"
#include "../include/construct/construction.h"
#include "../include/func/calculate_space.h"
#include "../include/func/print_structure.h"
#include "../params.h"
#include "./functions.h"

extern std::ofstream outRes;

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
               const carmi_params::TestDataVecType &testInsertQuery) {
  carmi_params::TestDataVecType init = initDataset;

#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "kAlgThreshold:" << thre << std::endl;
  std::cout << "-------------------------------" << std::endl;
  std::cout << "Start construction!" << std::endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << std::endl;
#endif

  double l = 0, r = 1;
  if (initRatio == carmi_params::kWritePartial) {
    l = 0.6;
    r = 0.9;
  }

  typedef CARMICommon<carmi_params::TestKeyType, carmi_params::TestValueType>
      CarmiType;
  CarmiType carmi(initDataset, initDataset.size(), initRatio, rate, thre, l, r);

#ifdef DEBUG
  time(&timep);
  char tmpTime1[64];
  strftime(tmpTime1, sizeof(tmpTime1), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "finish time: " << tmpTime1 << std::endl;

  std::cout << "\nprint the space:" << std::endl;
  auto space = carmi.CalculateSpace();
  outRes << space << ",";

  std::vector<int> levelVec(20, 0);
  std::vector<int> nodeVec(11, 0);
  std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
  std::cout << "print structure:" << std::endl;
  carmi.PrintStructure(1, NodeType(carmi.RootType()), 0, &levelVec, &nodeVec);
  std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;

  for (int i = 0; i < 20; i++) {
    if (levelVec[i] != 0)
      std::cout << "level " << i << ": " << levelVec[i] << std::endl;
    levelVec[i] = 0;
  }
  for (int i = 0; i < 11; i++) {
    if (nodeVec[i] != 0)
      std::cout << "node " << i << ": " << nodeVec[i] << std::endl;
    nodeVec[i] = 0;
  }
#endif

  if (initRatio == carmi_params::kWriteHeavy)
    WorkloadA<carmi_params::TestKeyType, carmi_params::TestValueType>(
        isZipfian, init, testInsertQuery,
        &carmi);  // write-heavy
  else if (initRatio == carmi_params::kReadHeavy)
    WorkloadB<carmi_params::TestKeyType, carmi_params::TestValueType>(
        isZipfian, init, testInsertQuery,
        &carmi);  // read-heavy
  else if (initRatio == carmi_params::kReadOnly)
    WorkloadC<carmi_params::TestKeyType, carmi_params::TestValueType>(
        isZipfian, init,
        &carmi);  // read-only
  else if (initRatio == carmi_params::kWritePartial)
    WorkloadD<carmi_params::TestKeyType, carmi_params::TestValueType>(
        isZipfian, init, testInsertQuery,
        &carmi);  // write-partial
  else if (initRatio == carmi_params::kRangeScan)
    WorkloadE<carmi_params::TestKeyType, carmi_params::TestValueType>(
        isZipfian, init, testInsertQuery, length,
        &carmi);  // range scan
}

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
                       const carmi_params::TestDataVecType &testInsertQuery) {
  carmi_params::TestDataVecType init = initDataset;

#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "kAlgThreshold:" << thre << std::endl;
  std::cout << "-------------------------------" << std::endl;
  std::cout << "Start construction!" << std::endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << std::endl;
#endif

  double l = 0, r = 1;
  if (initRatio == carmi_params::kWritePartial) {
    l = 0.6;
    r = 0.9;
  }

  carmi_params::TestKeyType *externalDataset;
  int record_size =
      sizeof(carmi_params::TestKeyType) + sizeof(carmi_params::TestValueType);
  typedef CARMIExternal<carmi_params::TestKeyType> CarmiType;
  int extLen =
      initDataset.size() * record_size / sizeof(carmi_params::TestKeyType) +
      carmi_params::kReservedSpace;
  externalDataset = new carmi_params::TestKeyType[extLen];
  for (int i = 0, j = 0; i < initDataset.size(); i++) {
    *(externalDataset + j) = initDataset[i].first;
    *(externalDataset + j + 1) = initDataset[i].second;
    j += 2;  // due to <double, double>
  }
  CarmiType carmi(externalDataset, initDataset.size(), extLen, record_size,
                  rate, thre);

  init.erase(init.begin() + carmi_params::kExternalInsertLeft, init.end());

#ifdef DEBUG
  time(&timep);
  char tmpTime1[64];
  strftime(tmpTime1, sizeof(tmpTime1), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "finish time: " << tmpTime1 << std::endl;

  std::cout << "\nprint the space:" << std::endl;
  auto space = carmi.CalculateSpace();
  outRes << space << ",";

  std::vector<int> levelVec(20, 0);
  std::vector<int> nodeVec(11, 0);
  std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
  std::cout << "print structure:" << std::endl;
  carmi.PrintStructure(1, NodeType(carmi.RootType()), 0, &levelVec, &nodeVec);
  std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;

  for (int i = 0; i < 20; i++) {
    if (levelVec[i] != 0)
      std::cout << "level " << i << ": " << levelVec[i] << std::endl;
    levelVec[i] = 0;
  }
  for (int i = 0; i < 11; i++) {
    if (nodeVec[i] != 0)
      std::cout << "node " << i << ": " << nodeVec[i] << std::endl;
    nodeVec[i] = 0;
  }
#endif

  if (initRatio == carmi_params::kWriteHeavy)
    WorkloadA<carmi_params::TestKeyType>(isZipfian, init, testInsertQuery,
                                         &carmi);  // write-heavy
  else if (initRatio == carmi_params::kReadHeavy)
    WorkloadB<carmi_params::TestKeyType>(isZipfian, init, testInsertQuery,
                                         &carmi);  // read-heavy
  else if (initRatio == carmi_params::kReadOnly)
    WorkloadC<carmi_params::TestKeyType>(isZipfian, init,
                                         &carmi);  // read-only
  else if (initRatio == carmi_params::kRangeScan)
    WorkloadE<carmi_params::TestKeyType>(isZipfian, init, testInsertQuery,
                                         length,
                                         &carmi);  // range scan
}

#endif  // SRC_EXPERIMENT_CORE_H_
