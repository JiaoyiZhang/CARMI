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

#include "../CARMI/carmi.h"
#include "../CARMI/construct/construction.h"
#include "../CARMI/func/calculate_space.h"
#include "../CARMI/func/print_structure.h"
#include "workload/workloads.h"

extern std::ofstream outRes;

void Core(double initRatio, double rate, int thre,
          const std::vector<int> &length,
          const std::vector<int> &trainInsertIndex, DataVectorType *initDataset,
          DataVectorType *trainFindQuery, DataVectorType *trainInsertQuery,
          DataVectorType *testInsertQuery) {
  for (int i = 0; i < trainFindQuery->size(); i++)
    (*trainFindQuery)[i].second = 1;
  for (int i = 0; i < trainInsertQuery->size(); i++)
    (*trainInsertQuery)[i].second = 1;

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

  CARMI carmi(*initDataset, *trainFindQuery, *trainInsertQuery,
              trainInsertIndex, rate, thre);
  if (kPrimaryIndex)
    initDataset->erase(initDataset->begin() + kExternalInsertLeft,
                       initDataset->end());

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
    WorkloadA(*initDataset, *testInsertQuery, &carmi);  // write-heavy
  else if (initRatio == kReadHeavy)
    WorkloadB(*initDataset, *testInsertQuery, &carmi);  // read-heavy
  else if (initRatio == kReadOnly)
    WorkloadC(*initDataset, &carmi);  // read-only
  else if (initRatio == kWritePartial)
    WorkloadD(*initDataset, *testInsertQuery, &carmi);  // write-partial
  else if (initRatio == kRangeScan)
    WorkloadE(*initDataset, *testInsertQuery, length, &carmi);  // range scan
}

#endif  // SRC_EXPERIMENT_CORE_H_
