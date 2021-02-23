#ifndef WORKLOAD_B_H
#define WORKLOAD_B_H
#include <vector>
#include "../../CARMI/carmi.h"
#include "../../CARMI/func/find_function.h"
#include "../../CARMI/func/insert_function.h"
#include "zipfian.h"
using namespace std;

extern ofstream outRes;

// read mostly workload
// a mix of 95/5 reads and writes
void WorkloadB(CARMI *carmi, vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset)
{
    auto init = initDataset;
    auto insert = insertDataset;

    default_random_engine engine;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed);
    shuffle(init.begin(), init.end(), engine);

    if (!kPrimaryIndex)
    {
        unsigned seed1 = chrono::system_clock::now().time_since_epoch().count();
        engine = default_random_engine(seed1);
        shuffle(insert.begin(), insert.end(), engine);
    }

    int end = 5000;
    int findCnt = 0;
    Zipfian zip;
    zip.InitZipfian(PARAM_ZIPFIAN, init.size());
    vector<int> index(95000, 0);
    for (int i = 0; i < 95000; i++)
    {
        int idx = zip.GenerateNextIndex();
        index[i] = idx;
    }

    chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = chrono::system_clock::now();
#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 19; j++)
        {
            carmi->Find(init[index[findCnt]].first);
            findCnt++;
        }
        carmi->Insert(insert[i]);
    }
#else
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 19 && findCnt < init.size(); j++)
        {
            carmi->Find(init[findCnt].first);
            findCnt++;
        }
        carmi->Insert(insert[i]);
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
            // init[index[findCnt]].first;
            findCnt++;
        }
        // insert[i];
    }
#else
    for (int i = 0; i < end; i++)
    {
        for (int j = 0; j < 19 && findCnt < init.size(); j++)
        {
            // init[findCnt].first;
            findCnt++;
        }
        // insert[i];
    }
#endif
    e = chrono::system_clock::now();
    double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    tmp -= tmp0;

    cout << "total time:" << tmp / 100000.0 * 1000000000 << endl;
    outRes << tmp / 100000.0 * 1000000000 << ",";
}

#endif // !WORKLOAD_B_H
