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

void CoreConstruct(double initRatio)
{
    cout << "kMaxKeyNum:" << kMaxKeyNum << "\tkRate:" << kRate << endl;
    findActualDataset = dataset;
    insertActualDataset = insertDataset;

    for (int i = 0; i < dataset.size(); i++)
        dataset[i].second = 2;
    for (int i = 0; i < insertDataset.size(); i++)
        insertDataset[i].second = 20;

    initEntireData(0, dataset.size() + insertDataset.size(), false);
    initEntireChild(dataset.size() + insertDataset.size());
    auto rootType = Construction(dataset, insertDataset);
    cout << "Construction over!" << endl;
    cout << endl;

    int size = 0;
    int cnt = 0;
    cout << "The index is:" << endl;
    cout << "LRVector: size: " << LRVector.size() << endl;
    cout << "NNVector: size: " << NNVector.size() << endl;
    cout << "HisVector: size: " << HisVector.size() << endl;
    cout << "BSVector: size: " << BSVector.size() << endl;
    cout << "ArrayVector size:" << ArrayVector.size() << endl;
    cout << "GAVector size:" << GAVector.size() << endl;
    cout << "the number of nodes: " << size << "\tcnt:" << cnt << endl;
    cout << "\nprint the space:" << endl;
    auto space = calculateSpace();
    outRes << space << ",";

    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    cout << "print structure:" << endl;
    printStructure(1, rootType, 0);
    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;

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