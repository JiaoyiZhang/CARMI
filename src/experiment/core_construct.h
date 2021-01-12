#ifndef CORE_CONSTRUCT_H
#define CORE_CONSTRUCT_H

#include "../construct/construction.h"

#include "../func/print_structure.h"
#include "../func/function.h"
#include "../func/calculate_space.h"

#include "../workload/workloada.h"
#include "../workload/workloadb.h"
#include "../workload/workloadc.h"
#include "../workload/workloadd.h"
using namespace std;

extern vector<pair<double, double>> findActualDataset;
extern vector<pair<double, double>> insertActualDataset;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

extern ofstream outRes;

vector<int> levelVec(20, 0);
vector<int> nodeVec(10, 0);

int totalFrequency = 0;

void CoreConstruct(double initRatio)
{
    cout << "kMaxKeyNum:" << kMaxKeyNum << "\tkRate:" << kRate << endl;
    findActualDataset = dataset;
    insertActualDataset = insertDataset;
    totalFrequency = 0;

    for (int i = 0; i < dataset.size(); i++)
    {
        dataset[i].second = 1;
        totalFrequency += 1;
    }
    for (int i = 0; i < insertDataset.size(); i++)
    {
        insertDataset[i].second = 1;
        totalFrequency += 1;
    }

    initEntireData(0, dataset.size() + insertDataset.size(), false);
    initEntireChild(dataset.size() + insertDataset.size());
    auto rootType = Construction(dataset, insertDataset);
    cout << "Construction over!" << endl;
    cout << endl;

    cout << "\nprint the space:" << endl;
    auto space = calculateSpace();
    outRes << space << ",";

    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    cout << "print structure:" << endl;
    printStructure(1, rootType, 0);
    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;

    for (int i = 0; i < 20; i++)
    {
        if (levelVec[i] != 0)
            cout << "level " << i << ": " << levelVec[i] << endl;
        levelVec[i] = 0;
    }
    for (int i = 0; i < 10; i++)
    {
        if (nodeVec[i] != 0)
            cout << "node " << i << ": " << nodeVec[i] << endl;
        nodeVec[i] = 0;
    }

    if (initRatio == 0.5)
        WorkloadA(rootType); // write-heavy
    else if (initRatio == 0.95)
        WorkloadB(rootType); // read-mostly
    else if (initRatio == 1)
        WorkloadC(rootType); // read-only
    else if (initRatio == 0)
        WorkloadD(rootType); // write-partially
}

#endif // !CORE_CONSTRUCT_H