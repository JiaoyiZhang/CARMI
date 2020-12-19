#ifndef BTREE_BASELINE_H
#define BTREE_BASELINE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include "../../stx_btree/btree_map.h"
using namespace std;

extern ofstream outRes;
extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

void btree_test(double initRatio)
{
    stx::btree_map<double, double> btree;
    for (int l = 0; l < dataset.size(); l++)
        btree.insert(dataset[l]);
    cout << "btree,";
    outRes << "btree,";

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(dataset.begin(), dataset.end(), default_random_engine(seed));
    shuffle(insertDataset.begin(), insertDataset.end(), default_random_engine(seed));

    if (initRatio == 0.5)
    {
        int end = min(dataset.size(), insertDataset.size());

        vector<uint64_t> rets;
        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            btree.find(dataset[i].first);
            btree.insert(insertDataset[i]);
        }
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
        }
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / float(dataset.size() + insertDataset.size()) * 1000000000 << endl;
        outRes << tmp / float(dataset.size() + insertDataset.size()) * 1000000000 << ",";
    }
    else if (initRatio == 0.95)
    {
        int end = insertDataset.size();
        int findCnt = 0;

        vector<uint64_t> rets;
        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                btree.find(dataset[findCnt].first);
                findCnt++;
            }
            btree.insert(insertDataset[i]);
        }
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        findCnt = 0;
        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                findCnt++;
            }
        }
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / float(dataset.size() + insertDataset.size()) * 1000000000 << endl;
        outRes << tmp / float(dataset.size() + insertDataset.size()) * 1000000000 << ",";
    }
    else if (initRatio == 1)
    {
        int end = dataset.size();

        vector<uint64_t> rets;
        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            btree.find(dataset[i].first);
        }
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
        }
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / float(dataset.size()) * 1000000000 << endl;
        outRes << tmp / float(dataset.size()) * 1000000000 << ",";
    }
    else if (initRatio == 0)
    {
        int end = insertDataset.size();
        int findCnt = 0;
        int insertCnt = 0;

        vector<uint64_t> rets;
        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
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
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        findCnt = 0;
        insertCnt = 0;
        s = chrono::system_clock::now();
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
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / float(dataset.size() + insertDataset.size()) * 1000000000 << endl;
        outRes << tmp / float(dataset.size() + insertDataset.size()) * 1000000000 << ",";
    }

    std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
    std::sort(insertDataset.begin(), insertDataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
}

#endif // !ART_TREE_BASELINE_H