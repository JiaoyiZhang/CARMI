#ifndef STATIC_H
#define STATIC_H

#include "workload/workloada.h"
#include "workload/workloadb.h"
#include "workload/workloadc.h"
#include "workload/workloadd.h"
#include "workload/workloade.h"

using namespace std;

extern int childNum;
extern ofstream outRes;

void RunStatic(double initRatio, vector<pair<double, double>> &initDataset,
               vector<pair<double, double>> &testInsertQuery,
               vector<int> &length, int kLeafID) {
  for (int j = 2; j < 3; j++) {
    cout << "root type:" << j << endl;
    CARMI carmi(initDataset, 131072, j, kLeafID);
    cout << "index init over!" << endl;
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

    if (initRatio == 0.5)
      WorkloadA(&carmi, initDataset, testInsertQuery);  // write-heavy
    else if (initRatio == 0.95)
      WorkloadB(&carmi, initDataset, testInsertQuery);  // read-heavy
    else if (initRatio == 1)
      WorkloadC(&carmi, initDataset);  // read-only
    else if (initRatio == 0)
      WorkloadD(&carmi, initDataset, testInsertQuery);  // write-partially
    else if (initRatio == 2)
      WorkloadE(&carmi, initDataset, testInsertQuery, length);  // range scan
  }
  outRes << endl;
}

#endif  // !STATIC_H
