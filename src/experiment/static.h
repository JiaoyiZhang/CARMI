/**
 * @file static.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_STATIC_H_
#define SRC_EXPERIMENT_STATIC_H_
#include <vector>

#include "../include/carmi.h"
#include "../include/staticRMI/rmi.h"
#include "workload/workloads.h"

extern int childNum;
extern std::ofstream outRes;

void RunStatic(double initRatio, int kLeafID, const DataVectorType &initDataset,
               const DataVectorType &testInsertQuery,
               const std::vector<int> &length) {
  for (int j = 2; j < 3; j++) {
    std::cout << "root type:" << j << std::endl;
    CARMI carmi(initDataset, 131072, j, kLeafID);
    std::cout << "index init over!" << std::endl;
    switch (j) {
      case LR_ROOT_NODE:
        outRes << "lr,";
        break;
      case PLR_ROOT_NODE:
        outRes << "plr,";
        break;
      case HIS_ROOT_NODE:
        outRes << "his,";
        break;
      case BS_ROOT_NODE:
        outRes << "bin,";
        break;
    }

    if (initRatio == kWriteHeavy)
      WorkloadA(initDataset, testInsertQuery, &carmi);  // write-heavy
    else if (initRatio == kReadHeavy)
      WorkloadB(initDataset, testInsertQuery, &carmi);  // read-heavy
    else if (initRatio == kReadOnly)
      WorkloadC(initDataset, &carmi);  // read-only
    else if (initRatio == kWritePartial)
      WorkloadD(initDataset, testInsertQuery, &carmi);  // write-partial
    else if (initRatio == kRangeScan)
      WorkloadE(initDataset, testInsertQuery, length, &carmi);  // range scan
  }
  outRes << std::endl;
}

#endif  // SRC_EXPERIMENT_STATIC_H_
