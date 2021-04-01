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

#include "../include/carmi.h"
#include "../include/construct/construction.h"
#include "../include/func/calculate_space.h"
#include "../include/func/print_structure.h"
#include "workload/workloads.h"

extern std::ofstream outRes;

void Core(double initRatio, double rate, int thre,
          const std::vector<int> &length,
          const std::vector<int> &trainInsertIndex,
          const DataVectorType &initDataset,
          const DataVectorType &trainFindQuery,
          const DataVectorType &trainInsertQuery,
          const DataVectorType &testInsertQuery) {
  DataVectorType init = initDataset;
  DataVectorType trainFind = trainFindQuery;
  DataVectorType trainInsert = trainInsertQuery;
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

  CARMI carmi(initDataset, trainFind, trainInsert, trainInsertIndex, rate,
              thre);
  if (kPrimaryIndex) {
    init.erase(init.begin() + kExternalInsertLeft, init.end());
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
  carmi.PrintStructure(1, NodeType(carmi.rootType), 0, &levelVec, &nodeVec);
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
    WorkloadA(init, testInsertQuery, &carmi);  // write-heavy
  else if (initRatio == kReadHeavy)
    WorkloadB(init, testInsertQuery, &carmi);  // read-heavy
  else if (initRatio == kReadOnly)
    WorkloadC(init, &carmi);  // read-only
  else if (initRatio == kWritePartial)
    WorkloadD(init, testInsertQuery, &carmi);  // write-partial
  else if (initRatio == kRangeScan)
    WorkloadE(init, testInsertQuery, length, &carmi);  // range scan
}

#endif  // SRC_EXPERIMENT_CORE_H_
