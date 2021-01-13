#ifndef ART_TREE_BASELINE_H
#define ART_TREE_BASELINE_H

#include "../../art_tree/art.h"
#include "../../art_tree/art.cpp"
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>
#include <stdio.h>
using namespace std;

extern ofstream outRes;
extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

void artTree_test(double initRatio)
{
    outRes << "artTree,";
    cout << "artTree,";
    art_tree t;
    art_tree_init(&t);
    cout<<"start"<<endl;
    for (int i = 0; i < dataset.size(); i++)
    {
        char key[64];
        sprintf(key, "%f", dataset[i].first);
        art_insert(&t, (const unsigned char *)key, strlen((const char *)key), (uint64_t)dataset[i].second);
    }

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(dataset.begin(), dataset.end(), default_random_engine(seed));
    shuffle(insertDataset.begin(), insertDataset.end(), default_random_engine(seed));
    cout << "shuffle over" << endl;

    if (initRatio == 0.5)
    {
        int end = min(dataset.size(), insertDataset.size());

        vector<uint64_t> rets;
        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            char key[64];
            sprintf(key, "%f", dataset[i].first);
            art_search(&t, (const unsigned char *)key, strlen((const char *)key), rets);
            sprintf(key, "%f", insertDataset[i].first);
            art_insert(&t, (const unsigned char *)key, strlen((const char *)key), (uint64_t)insertDataset[i].second);
        }
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            char key[64];
            sprintf(key, "%f", dataset[i].first);
            sprintf(key, "%f", insertDataset[i].first);
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
                char key[64];
                sprintf(key, "%f", dataset[findCnt].first);
                art_search(&t, (const unsigned char *)key, strlen((const char *)key), rets);
                findCnt++;
            }
            char key[64];
            sprintf(key, "%f", insertDataset[i].first);
            art_insert(&t, (const unsigned char *)key, strlen((const char *)key), (uint64_t)insertDataset[i].second);
        }
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        findCnt = 0;
        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                char key[64];
                sprintf(key, "%f", dataset[findCnt].first);
                findCnt++;
            }
            char key[64];
            sprintf(key, "%f", insertDataset[i].first);
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
            char key[64];
            sprintf(key, "%f", dataset[i].first);
            art_search(&t, (const unsigned char *)key, strlen((const char *)key), rets);
        }
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            char key[64];
            sprintf(key, "%f", dataset[i].first);
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
                char key[64];
                sprintf(key, "%f", dataset[findCnt].first);
                art_search(&t, (const unsigned char *)key, strlen((const char *)key), rets);
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                char key[64];
                sprintf(key, "%f", insertDataset[insertCnt].first);
                art_insert(&t, (const unsigned char *)key, strlen((const char *)key), (uint64_t)insertDataset[insertCnt].second);
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
                char key[64];
                sprintf(key, "%f", dataset[findCnt].first);
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                char key[64];
                sprintf(key, "%f", insertDataset[insertCnt].first);
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