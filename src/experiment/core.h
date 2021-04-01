/**
 * @file core.h
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
#include "workload/workloads.h"

extern std::ofstream outRes;

void Core(double initRatio, double rate, int thre,
          const std::vector<int> &length,
          const std::vector<int> &trainInsertIndex,
          const carmi_params::TestDataVecType &initDataset,
          const carmi_params::TestDataVecType &trainFindQuery,
          const carmi_params::TestDataVecType &trainInsertQuery,
          const carmi_params::TestDataVecType &testInsertQuery) {
  carmi_params::TestDataVecType init = initDataset;
  carmi_params::TestDataVecType trainFind = trainFindQuery;
  carmi_params::TestDataVecType trainInsert = trainInsertQuery;
  for (int i = 0; i < trainFind.size(); i++) {
    trainFind[i].second = 1;
  }
  for (int i = 0; i < trainInsert.size(); i++) {
    trainInsert[i].second = 1;
  }

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
  int record_size =
      sizeof(carmi_params::TestKeyType) + sizeof(carmi_params::TestValueType);
  carmi_params::TestKeyType *externalDataset;
  if (carmi_params::kPrimaryIndex) {
    int extLen =
        initDataset.size() * record_size / sizeof(carmi_params::TestKeyType);
    externalDataset = new carmi_params::TestKeyType[extLen];
    for (int i = 0, j = 0; i < initDataset.size(); i++) {
      *(externalDataset + j) = initDataset[i].first;
      *(externalDataset + j + 1) = initDataset[i].second;
      j += 2;  // due to <double, double>
    }
  }

#ifdef EXTERNAL
  CARMIExternal<carmi_params::TestKeyType> carmi(
      externalDataset, initDataset.size(), record_size, initRatio, rate, thre);
#else
  CARMICommon<carmi_params::TestKeyType, carmi_params::TestValueType> carmi(
      initDataset, initDataset.size(), initRatio, rate, thre, l, r);
#endif

  // CARMI<carmi_params::TestKeyType, carmi_params::TestValueType> carmi(
  //     initDataset, trainFind, trainInsert, trainInsertIndex, rate, thre);
  if (carmi_params::kPrimaryIndex) {
    init.erase(init.begin() + carmi_params::kExternalInsertLeft, init.end());
  }

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
#ifdef EXTERNAL
  typedef CARMIExternal<carmi_params::TestKeyType> CarmiType;
#else
  typedef CARMICommon<carmi_params::TestKeyType, carmi_params::TestValueType>
      CarmiType;
#endif
  // zipfian
  if (initRatio == carmi_params::kWriteHeavy)
    WorkloadA<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(true, init, testInsertQuery,
                                           &carmi);  // write-heavy
  else if (initRatio == carmi_params::kReadHeavy)
    WorkloadB<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(true, init, testInsertQuery,
                                           &carmi);  // read-heavy
  else if (initRatio == carmi_params::kReadOnly)
    WorkloadC<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(true, init, &carmi);  // read-only
  else if (initRatio == carmi_params::kWritePartial)
    WorkloadD<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(true, init, testInsertQuery,
                                           &carmi);  // write-partial
  else if (initRatio == carmi_params::kRangeScan)
    WorkloadE<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(true, init, testInsertQuery, length,
                                           &carmi);  // range scan

  // uniform
  if (initRatio == carmi_params::kWriteHeavy)
    WorkloadA<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(false, init, testInsertQuery,
                                           &carmi);  // write-heavy
  else if (initRatio == carmi_params::kReadHeavy)
    WorkloadB<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(false, init, testInsertQuery,
                                           &carmi);  // read-heavy
  else if (initRatio == carmi_params::kReadOnly)
    WorkloadC<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(false, init, &carmi);  // read-only
  else if (initRatio == carmi_params::kWritePartial)
    WorkloadD<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(false, init, testInsertQuery,
                                           &carmi);  // write-partial
  else if (initRatio == carmi_params::kRangeScan)
    WorkloadE<CarmiType, carmi_params::TestKeyType,
              carmi_params::TestValueType>(false, init, testInsertQuery, length,
                                           &carmi);  // range scan
}

#endif  // SRC_EXPERIMENT_CORE_H_
