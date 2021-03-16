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
#include "workload/workloada.h"
#include "workload/workloadb.h"
#include "workload/workloadc.h"
#include "workload/workloadd.h"
#include "workload/workloade.h"

extern ofstream outRes;

void Core(double initRatio, double rate, int thre, const vector<int> &length,
          DataVectorType *initDataset, DataVectorType *trainFindQuery,
          DataVectorType *trainInsertQuery, DataVectorType *testInsertQuery) {
  for (int i = 0; i < trainFindQuery->size(); i++)
    (*trainFindQuery)[i].second = 1;
  for (int i = 0; i < trainInsertQuery->size(); i++)
    (*trainInsertQuery)[i].second = 1;

#ifdef DEBUG
  std::cout << endl;
  std::cout << "kAlgThreshold:" << thre << endl;
  std::cout << "-------------------------------" << endl;
  std::cout << "Start construction!" << endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << endl;
#endif

  CARMI carmi(*initDataset, *trainFindQuery, *trainInsertQuery, rate, thre);

#ifdef DEBUG
  time(&timep);
  char tmpTime1[64];
  strftime(tmpTime1, sizeof(tmpTime1), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "finish time: " << tmpTime1 << endl;

  std::cout << "\nprint the space:" << endl;
  auto space = carmi.calculateSpace();
  outRes << space << ",";

  vector<int> levelVec(20, 0);
  vector<int> nodeVec(11, 0);
  std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
  std::cout << "print structure:" << std::endl;
  carmi.PrintStructure(1, NodeType(carmi.rootType), 0, &levelVec, &nodeVec);
  std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;

  for (int i = 0; i < 20; i++) {
    if (levelVec[i] != 0) cout << "level " << i << ": " << levelVec[i] << endl;
    levelVec[i] = 0;
  }
  for (int i = 0; i < 11; i++) {
    if (nodeVec[i] != 0) cout << "node " << i << ": " << nodeVec[i] << endl;
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
