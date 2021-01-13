#ifndef WORKLOAD_C_H
#define WORKLOAD_C_H
#include <vector>
#include "../func/function.h"
#include "zipfian.h"
using namespace std;

extern vector<pair<double, double>> findActualDataset;
extern vector<pair<double, double>> insertActualDataset;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

extern ofstream outRes;

// read only workload
// 100% read
void WorkloadC(int rootType)
{
    dataset = findActualDataset;

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

    default_random_engine engine;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed);
    shuffle(dataset.begin(), dataset.end(), engine);

    int end = dataset.size();
    Zipfian zipFind;
    zipFind.InitZipfian(PARAM_ZIPFIAN, dataset.size());
    vector<int> index;
    for (int i = 0; i < end; i++)
    {
        int idx = zipFind.GenerateNextIndex();
        index.push_back(idx);
    }

    chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = chrono::system_clock::now();
#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        Find(rootType, dataset[index[i]].first);
    }
#else
    for (int i = 0; i < end; i++)
    {
        Find(rootType, dataset[i].first);
    }
#endif
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

    s = chrono::system_clock::now();

#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        TestFind(rootType, dataset[index[i]].first);
    }
#else
    for (int i = 0; i < end; i++)
    {
        TestFind(rootType, dataset[i].first);
    }
#endif
    e = chrono::system_clock::now();
    double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "tmp:" << tmp << endl;
    cout << "tmp0:" << tmp0 << endl;
    tmp -= tmp0;

    cout << "total time:" << tmp / float(dataset.size()) * 1000000000 << endl;

    outRes << tmp / float(dataset.size()) * 1000000000 << ",";

    std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
    std::sort(insertDataset.begin(), insertDataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
}

#endif // !WORKLOAD_C_H
