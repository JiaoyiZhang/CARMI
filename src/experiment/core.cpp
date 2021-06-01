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
#include <vector>

#include "../include/carmi_common.h"
#include "../include/carmi_external.h"
#include "../include/construct/construction.h"
#include "../include/func/calculate_space.h"
#include "../include/func/print_structure.h"
#include "./experiment_params.h"
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
void CoreCARMI(bool isZipfian, double initRatio, double rate,
               const std::vector<int> &length, const DataVecType &initDataset,
               const DataVecType &testInsertQuery) {
  DataVecType init = initDataset;

#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "-------------------------------" << std::endl;
  std::cout << "Start construction!" << std::endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << std::endl;
#endif

  double l = 0, r = 1;
  if (initRatio == kWritePartial) {
    l = 0.6;
    r = 0.9;
  }
  double initR = initRatio;
  if (initR == kRangeScan) {
    initR = kReadHeavy;
  }

  typedef CARMICommon<KeyType, ValueType> CarmiType;
  CarmiType carmi(initDataset.begin(), initDataset.end(), initR, rate, l, r);

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
  carmi.PrintStructure(1, NodeType(0), 0, &levelVec, &nodeVec);
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

  if (initRatio == kWriteHeavy)
    WorkloadA<KeyType, ValueType>(isZipfian, init, testInsertQuery,
                                  &carmi);  // write-heavy
  else if (initRatio == kReadHeavy)
    WorkloadB<KeyType, ValueType>(isZipfian, init, testInsertQuery,
                                  &carmi);  // read-heavy
  else if (initRatio == kReadOnly)
    WorkloadC<KeyType, ValueType>(isZipfian, init,
                                  &carmi);  // read-only
  else if (initRatio == kWritePartial)
    WorkloadD<KeyType, ValueType>(isZipfian, init, testInsertQuery,
                                  &carmi);  // write-partial
  else if (initRatio == kRangeScan)
    WorkloadE<KeyType, ValueType>(isZipfian, init, testInsertQuery, length,
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
void CoreExternalCARMI(bool isZipfian, double initRatio, double rate,
                       const std::vector<int> &length,
                       const DataVecType &initDataset,
                       const DataVecType &testInsertQuery) {
  DataVecType init = initDataset;

#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "-------------------------------" << std::endl;
  std::cout << "Start construction!" << std::endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << std::endl;
#endif

  double l = 0, r = 1;
  if (initRatio == kWritePartial) {
    l = 0.6;
    r = 0.9;
  }

  KeyType *externalDataset;
  const int record_size = sizeof(KeyType) + sizeof(ValueType);
  typedef CARMIExternal<KeyType> CarmiType;
  int extLen = initDataset.size() * 2 + kTestSize * 2;
  externalDataset = new KeyType[extLen];
  for (int i = 0, j = 0; i < initDataset.size(); i++) {
    *(externalDataset + j) = initDataset[i].first;
    *(externalDataset + j + 1) = initDataset[i].second;
    j += 2;  // due to <double, double>
  }
  std::vector<KeyType> futureInsertKey(testInsertQuery.size(), 0);
  for (int i = 0; i < testInsertQuery.size(); i++) {
    futureInsertKey[i] = testInsertQuery[i].first;
  }
  // initDataset -> only includes the findQuery
  CarmiType carmi(externalDataset, futureInsertKey, initDataset.size(),
                  record_size, rate);

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
  carmi.PrintStructure(1, NodeType(0), 0, &levelVec, &nodeVec);
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

  if (initRatio == kWriteHeavy)
    WorkloadA<KeyType>(isZipfian, init, testInsertQuery,
                       &carmi);  // write-heavy
  else if (initRatio == kReadHeavy)
    WorkloadB<KeyType>(isZipfian, init, testInsertQuery,
                       &carmi);  // read-heavy
  else if (initRatio == kReadOnly)
    WorkloadC<KeyType>(isZipfian, init,
                       &carmi);  // read-only
  else if (initRatio == kRangeScan)
    WorkloadE<KeyType>(isZipfian, init, testInsertQuery, length,
                       &carmi);  // range scan
}
