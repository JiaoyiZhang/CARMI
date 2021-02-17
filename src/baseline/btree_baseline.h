#ifndef BTREE_BASELINE_H
#define BTREE_BASELINE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include "../../stx_btree/btree_map.h"
#include "../params.h"
#include "../experiment/workload/zipfian.h"
using namespace std;

extern ofstream outRes;
extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;
extern vector<int> length;

void btree_test(double initRatio)
{
    cout<<"dataset size:"<<dataset.size()<<endl;
    stx::btree_map<double, double> btree(dataset.begin(), dataset.end()); // vector begin-> end
    auto stat = btree.get_stats();
    cout << "btree : innernodes:" << stat.innernodes << "，\tleaf nodes:" << stat.leaves << endl;
    double space = float(stat.innernodes * 272 + stat.leaves * 280) / 1024 / 1024;
    cout << "btree space:" << space << endl;
    // for (int l = 0; l < dataset.size(); l++)
    //     btree.insert(dataset[l]);
    cout << "btree,";
    outRes << "btree,";

    default_random_engine engine;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed);
    shuffle(dataset.begin(), dataset.end(), engine);

    unsigned seed1 = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed1);
    shuffle(insertDataset.begin(), insertDataset.end(), engine);

    Zipfian zipFind;
    zipFind.InitZipfian(PARAM_ZIPFIAN, dataset.size());
    vector<int> index;
    for (int i = 0; i < dataset.size(); i++)
    {
        int idx = zipFind.GenerateNextIndex();
        index.push_back(idx);
    }

    if (initRatio == 0.5)
    {
        int end = 50000;
        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            btree.find(dataset[index[i]].first);
            btree.insert(insertDataset[i]);
        }
#else
        for (int i = 0; i < end; i++)
        {
            btree.find(dataset[i].first);
            btree.insert(insertDataset[i]);
        }
#endif
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
        }
#else
        for (int i = 0; i < end; i++)
        {
        }
#endif
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / 100000.0 * 1000000000 << endl;
        outRes << tmp / 100000.0 * 1000000000 << ",";
    }
    else if (initRatio == 0.95)
    {
        int end = 5000;
        int findCnt = 0;

        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                btree.find(dataset[index[findCnt]].first);
                findCnt++;
            }
            btree.insert(insertDataset[i]);
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                btree.find(dataset[findCnt].first);
                findCnt++;
            }
            btree.insert(insertDataset[i]);
        }
#endif
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        findCnt = 0;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                findCnt++;
            }
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                findCnt++;
            }
        }
#endif
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / 100000.0 * 1000000000 << endl;
        outRes << tmp / 100000.0 * 1000000000 << ",";
    }
    else if (initRatio == 1)
    {
        int end = 100000;

        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            btree.find(dataset[index[i]].first);
        }
#else
        for (int i = 0; i < end; i++)
        {
            btree.find(dataset[i].first);
        }
#endif
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
        }
#else
        for (int i = 0; i < end; i++)
        {
        }
#endif
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / 100000.0 * 1000000000 << endl;
        outRes << tmp / 100000.0 * 1000000000 << ",";
    }
    else if (initRatio == 0)
    {
        int end = 15000;
        int findCnt = 0;
        int insertCnt = 0;

        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 17 && findCnt < dataset.size(); j++)
            {
                btree.find(dataset[index[findCnt]].first);
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                btree.insert(insertDataset[insertCnt]);
                insertCnt++;
            }
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 17 && findCnt < dataset.size(); j++)
            {
                btree.find(dataset[findCnt].first);
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                btree.insert(insertDataset[insertCnt]);
                insertCnt++;
            }
        }
#endif
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        findCnt = 0;
        insertCnt = 0;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 17 && findCnt < dataset.size(); j++)
            {
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                insertCnt++;
            }
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 17 && findCnt < dataset.size(); j++)
            {
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                insertCnt++;
            }
        }
#endif
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / 100000.0 * 1000000000 << endl;
        outRes << tmp / 100000.0 * 1000000000 << ",";
    }
    else if (initRatio == 2)
    {
        int end = 5000;
        int findCnt = 0;

        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                vector<pair<double, double>> ret(length[index[findCnt]], {-1, -1});
                auto it = btree.find(dataset[index[findCnt]].first);
                for (int l = 0; l < length[index[findCnt]]; l++)
                {
                    ret[l] = {it->first, it->second};
                    it++;
                }
                findCnt++;
            }
            btree.insert(insertDataset[i]);
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                vector<pair<double, double>> ret(length[findCnt], {-1, -1});
                auto it = btree.find(dataset[findCnt].first);
                for (int l = 0; l < length[findCnt]; l++)
                {
                    ret[l] = {it->first, it->second};
                    it++;
                }
                findCnt++;
            }
            btree.insert(insertDataset[i]);
        }
#endif
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        findCnt = 0;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                vector<pair<double, double>> ret(length[index[findCnt]], {-1, -1});
                stx::btree<double, double>::iterator it;
                for (int l = 0; l < length[index[findCnt]]; l++)
                {
                }
                findCnt++;
            }
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                vector<pair<double, double>> ret(length[findCnt], {-1, -1});
                stx::btree<double, double>::iterator it;
                for (int l = 0; l < length[findCnt]; l++)
                {
                }
                findCnt++;
            }
        }
#endif
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / 100000.0 * 1000000000 << endl;
        outRes << tmp / 100000.0 * 1000000000 << ",";
    }

    // auto stat = btree.get_stats();
    // cout << "btree : innernodes" << stat.innernodes << "，\tleaf nodes:" << stat.leaves << endl;
    // double space = float(stat.innernodes * 272 + stat.leaves * 280) / 1024 / 1024;
    cout << "btree space:" << space << endl;
    // space += float(dataset.size() + insertDataset.size()) * 16 / 1024 / 1024;
    // cout << "btree total space: " << space << endl;
    outRes << space << endl;

    std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
    std::sort(insertDataset.begin(), insertDataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
}

#endif // !ART_TREE_BASELINE_H