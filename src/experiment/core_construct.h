#ifndef CORE_CONSTRUCT_H
#define CORE_CONSTRUCT_H

#include "../construct/construction.h"

#include "../func/print_structure.h"
#include "../func/function.h"
#include "../func/calculate_space.h"
using namespace std;

extern vector<pair<double, double>> findActualDataset;
extern vector<pair<double, double>> insertActualDataset;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

extern ofstream outRes;

void CoreConstruct()
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

    dataset = findActualDataset;
    insertDataset = insertActualDataset;

    if (kRate == 1)
    {
        for (int i = 0; i < dataset.size(); i++)
        {
            auto res = Find(rootType, dataset[i].first);
            // if ((res.second != dataset[i].second) || (res.first != dataset[i].first))
            if (res.first != dataset[i].first)
                cout << "Find failed:\ti:" << i << "\tdata:" << dataset[i].first << "\t" << dataset[i].second << "\tres: " << res.first << "\t" << res.second << endl;
        }
        cout << "check FIND over!" << endl;
    }

    // chrono::_V2::system_clock::time_point s, e;
    double tmp;
    // s = chrono::system_clock::now();
    for (int i = 0; i < dataset.size(); i++)
        Find(rootType, dataset[i].first);
    // e = chrono::system_clock::now();
    // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "Find time:" << tmp / (float)dataset.size() * 1000000000 - 5 << endl;
    outRes << tmp / (float)dataset.size() * 1000000000 - 5 << ",";

    // s = chrono::system_clock::now();
    for (int i = 0; i < insertDataset.size(); i++)
        Insert(rootType, insertDataset[i]);
    // e = chrono::system_clock::now();
    // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "Insert time:" << tmp / (float)insertDataset.size() * 1000000000 - 5 << endl;
    outRes << tmp / (float)insertDataset.size() * 1000000000 - 5 << "ns,";
}





#endif // !CORE_CONSTRUCT_H