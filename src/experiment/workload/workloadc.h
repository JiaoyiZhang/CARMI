#ifndef WORKLOAD_C_H
#define WORKLOAD_C_H
#include <vector>
#include "zipfian.h"
#include "../../CARMI/carmi.h"
using namespace std;
extern ofstream outRes;

// read only workload
// 100% read
void WorkloadC(CARMI carmi, vector<pair<double, double>> &initDataset)
{
    auto init = initDataset;

    default_random_engine engine;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed);
    shuffle(init.begin(), init.end(), engine);

    int end = 100000;
    Zipfian zipFind;
    zipFind.InitZipfian(PARAM_ZIPFIAN, init.size());
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
        carmi.Find(init[index[i]].first);
    }
#else
    for (int i = 0; i < end; i++)
    {
        carmi.Find(init[i].first);
    }
#endif
    e = chrono::system_clock::now();
    tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

    s = chrono::system_clock::now();

#if ZIPFIAN
    for (int i = 0; i < end; i++)
    {
        init[index[i]].first;
    }
#else
    for (int i = 0; i < end; i++)
    {
        init[i].first;
    }
#endif
    e = chrono::system_clock::now();
    double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    tmp -= tmp0;

    cout << "total time:" << tmp / 100000.0 * 1000000000 << endl;

    outRes << tmp / 100000.0 * 1000000000 << ",";
}

#endif // !WORKLOAD_C_H
