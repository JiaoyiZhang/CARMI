#ifndef WORKLOAD_A_H
#define WORKLOAD_A_H
#include <vector>
#include "../func/function.h"
#include "zipfian.h"
using namespace std;

extern vector<pair<double, double>> findActualDataset;
extern vector<pair<double, double>> insertActualDataset;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

extern ofstream outRes;

// update heavy workload
// a mix of 50/50 reads and writes
void WorkloadA(int rootType)
{
    dataset = findActualDataset;
    insertDataset = insertActualDataset;

    // if (kRate == 1)
    // {
    for (int i = 0; i < dataset.size(); i++)
    {
        auto res = Find(rootType, dataset[i].first);
        if (res.first != dataset[i].first)
            cout << "Find failed:\ti:" << i << "\tdata:" << dataset[i].first << "\t" << dataset[i].second << "\tres: " << res.first << "\t" << res.second << endl;
    }
    cout << "check FIND over!" << endl;
    // }

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(dataset.begin(), dataset.end(), default_random_engine(seed));
    shuffle(insertDataset.begin(), insertDataset.end(), default_random_engine(seed));

    int end = min(dataset.size(), insertDataset.size());
    Zipfian zip;
    zip.InitZipfian(PARAM_ZIPFIAN, end);

    chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = chrono::system_clock::now();
#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        int idx = zip.GenerateNextIndex();
        Find(rootType, dataset[idx].first);
        Insert(rootType, insertDataset[i]);
    }
#else
    for (int i = 0; i < end; i++)
    {
        Find(rootType, dataset[i].first);
        Insert(rootType, insertDataset[i]);
    }
#endif
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

    s = chrono::system_clock::now();
#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        int idx = zip.GenerateNextIndex();
        TestFind(rootType, dataset[idx].first);
        TestFind(rootType, insertDataset[i].first);
    }
#else
    for (int i = 0; i < end; i++)
    {
        TestFind(rootType, dataset[i].first);
        TestFind(rootType, insertDataset[i].first);
    }
#endif
    e = chrono::system_clock::now();
    double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    tmp -= tmp0;

    cout << "total time:" << tmp / float(dataset.size() + insertDataset.size()) * 1000000000 << endl;

    outRes << tmp / float(dataset.size() + insertDataset.size()) * 1000000000 << ",";

    std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
    std::sort(insertDataset.begin(), insertDataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
}

#endif // !WORKLOAD_A_H
