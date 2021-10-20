/**
 * @file core.cpp
 * @author Jiaoyi
 * @brief
 * @version 3.0
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
 * @param length the length of range scan
 * @param initDataset
 * @param testInsertQuery
 */
void CoreCARMI(bool isZipfian, double initRatio, double rate,
               const std::vector<int> &length, const DataVecType &initDataset,
               const DataVecType &insertDataset,
               const DataVecType &testInsertQuery) {
  DataVecType init = initDataset;

#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "-------------------------------" << std::endl;
  std::cout << "kRate: " << rate << std::endl;
  std::cout << "Start construction!" << std::endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << std::endl;
#endif

  typedef CARMICommon<KeyType, ValueType> CarmiType;
  CarmiType carmi(initDataset.begin(), initDataset.end(), insertDataset.begin(),
                  insertDataset.end(), rate);

#ifdef DEBUG
  time(&timep);
  char tmpTime1[64];
  strftime(tmpTime1, sizeof(tmpTime1), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "finish time: " << tmpTime1 << std::endl;

  std::cout << "\nprint the space:" << std::endl;
  auto space = carmi.CalculateSpace();
  outRes << space << ",";
  std::cout << space << " MB\n";
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

  KeyType *externalDataset;
  const int record_size = sizeof(KeyType) + sizeof(ValueType);
  typedef CARMIExternal<KeyType> CarmiType;
  int extLen = initDataset.size() * 2 + kTestSize * 2;
  externalDataset = new KeyType[extLen];
  for (int i = 0, j = 0; i < static_cast<int>(initDataset.size()); i++) {
    *(externalDataset + j) = initDataset[i].first;
    *(externalDataset + j + 1) = initDataset[i].second;
    j += 2;  // due to <double, double>
  }
  std::vector<KeyType> futureInsertKey(testInsertQuery.size(), 0);
  for (int i = 0; i < static_cast<int>(testInsertQuery.size()); i++) {
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
