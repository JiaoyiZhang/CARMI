#ifndef CORE_H
#define CORE_H

#include "../CARMI/construct/construction.h"
#include "../CARMI/func/calculate_space.h"
#include "../CARMI/func/print_structure.h"
#include "../CARMI/carmi.h"

#include "workload/workloada.h"
#include "workload/workloadb.h"
#include "workload/workloadc.h"
#include "workload/workloadd.h"
#include "workload/workloade.h"
using namespace std;

extern ofstream outRes;

void Core(double initRatio, vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery, vector<int> &length, double rate)
{
    for (int i = 0; i < trainFindQuery.size(); i++)
        trainFindQuery[i].second = 1;
    for (int i = 0; i < trainInsertQuery.size(); i++)
        trainInsertQuery[i].second = 1;

    CARMI carmi(initDataset, trainFindQuery, trainInsertQuery, rate);
    cout << "Construction over!" << endl;
    cout << endl;

    cout << "\nprint the space:" << endl;
    auto space = carmi.calculateSpace();
    outRes << space << ",";

    vector<int> levelVec(20, 0);
    vector<int> nodeVec(11, 0);
    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    cout << "print structure:" << endl;
    carmi.printStructure(levelVec, nodeVec, 1, carmi.rootType, 0);
    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;

    for (int i = 0; i < 20; i++)
    {
        if (levelVec[i] != 0)
            cout << "level " << i << ": " << levelVec[i] << endl;
        levelVec[i] = 0;
    }
    for (int i = 0; i < 11; i++)
    {
        if (nodeVec[i] != 0)
            cout << "node " << i << ": " << nodeVec[i] << endl;
        nodeVec[i] = 0;
    }

    if (initRatio == 0.5)
        WorkloadA(&carmi, initDataset, testInsertQuery); // write-heavy
    else if (initRatio == 0.95)
        WorkloadB(&carmi, initDataset, testInsertQuery); // read-heavy
    else if (initRatio == 1)
        WorkloadC(&carmi, initDataset); // read-only
    else if (initRatio == 0)
        WorkloadD(&carmi, initDataset, testInsertQuery); // write-partial
    else if (initRatio == 2)
        WorkloadE(&carmi, initDataset, testInsertQuery, length); // range scan
}

#endif // !CORE_H