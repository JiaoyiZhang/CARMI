#ifndef WORKLOAD_B_H
#define WORKLOAD_B_H
#include <vector>
#include "../func/function.h"
#include "zipfian.h"
using namespace std;

extern vector<pair<double, double>> findActualDataset;
extern vector<pair<double, double>> insertActualDataset;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

extern ofstream outRes;

// read mostly workload
// a mix of 95/5 reads and writes
void WorkloadB(int rootType)
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
    default_random_engine engine;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed);
    shuffle(dataset.begin(), dataset.end(), engine);

    unsigned seed1 = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed1);
    shuffle(insertDataset.begin(), insertDataset.end(), engine);

    int end = insertDataset.size();
    int findCnt = 0;
    Zipfian zipFind;
    zipFind.InitZipfian(PARAM_ZIPFIAN, dataset.size());
    vector<int> index;
    for (int i = 0; i < dataset.size(); i++)
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
        for (int j = 0; j < 19; j++)
        {
            Find(rootType, dataset[index[findCnt]].first);
            findCnt++;
        }
        Insert(rootType, insertDataset[i]);
    }
#else
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
        {
            Find(rootType, dataset[findCnt].first);
            findCnt++;
        }
        Insert(rootType, insertDataset[i]);
    }
#endif
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

    findCnt = 0;
    s = chrono::system_clock::now();
#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 19; j++)
        {
            TestFind(rootType, dataset[index[findCnt]].first);
            findCnt++;
        }
        TestFind(rootType, insertDataset[i].first);
    }
#else
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
        {
            TestFind(rootType, dataset[findCnt].first);
            findCnt++;
        }
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

#endif // !WORKLOAD_B_H
