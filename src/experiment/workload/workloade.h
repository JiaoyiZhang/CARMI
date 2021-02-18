#ifndef WORKLOAD_E_H
#define WORKLOAD_E_H
#include <vector>
#include "zipfian.h"
using namespace std;

extern ofstream outRes;

// read mostly workload (range scan)
// a mix of 95/5 reads and writes
void WorkloadE(CARMI carmi, vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset, vector<int> length)
{
    auto init = initDataset;
    auto insert = insertDataset;

    default_random_engine engine;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed);
    shuffle(init.begin(), init.end(), engine);

    unsigned seed1 = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed1);
    shuffle(insert.begin(), insert.end(), engine);

    int end = 5000;
    int findCnt = 0;
    Zipfian zipFind;
    zipFind.InitZipfian(PARAM_ZIPFIAN, init.size());
    vector<int> index;
    for (int i = 0; i < init.size(); i++)
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
        for (int j = 0; j < 19 && findCnt < init.size(); j++)
        {
            vector<pair<double, double>> ret(length[findCnt], {-1, -1});
            carmi.RangeScan(init[index[findCnt]].first, length[findCnt], ret);
            findCnt++;
        }
        carmi.Insert(insert[i]);
    }
#else
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 19 && findCnt < init.size(); j++)
        {
            vector<pair<double, double>> ret(length[findCnt], {-1, -1});
            carmi.RangeScan(init[findCnt].first, length[findCnt], ret);
            findCnt++;
        }
        carmi.Insert(insert[i]);
    }
#endif
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "over!" << endl;

    findCnt = 0;
    s = chrono::system_clock::now();
#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 19 && findCnt < init.size(); j++)
        {
            vector<pair<double, double>> ret(length[findCnt], {-1, -1});
            init[index[findCnt]].first;
            findCnt++;
        }
        insert[i];
    }
#else
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 19 && findCnt < init.size(); j++)
        {
            vector<pair<double, double>> ret(length[findCnt], {-1, -1});
            init[findCnt].first;
            findCnt++;
        }
        insert[i];
    }
#endif
    e = chrono::system_clock::now();
    double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    tmp -= tmp0;

    cout << "total time:" << tmp / 100000.0 * 1000000000 << endl;
    outRes << tmp / 100000.0 * 1000000000 << ",";
}

#endif // !WORKLOAD_B_H
