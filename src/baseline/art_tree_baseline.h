#ifndef ART_TREE_BASELINE_H
#define ART_TREE_BASELINE_H

#include "../../art_tree/art.h"
#include "../../art_tree/art.cpp"
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>
#include <stdio.h>
#include "../params.h"
#include "../workload/zipfian.h"
#include "iostream"
using namespace std;

extern ofstream outRes;
extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;
extern vector<int> length;

void artTree_test(double initRatio)
{
    outRes << "artTree,";
    cout << "artTree,";
    art_tree t;
    art_tree_init(&t);
    cout << "start" << endl;
    for (int i = 0; i < dataset.size(); i++)
    {
        char key[64] = {0};
        sprintf(key, "%f", dataset[i].first);
        art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1, dataset[i].second);
    }
    cout << "init over" << endl;

    default_random_engine engine;

    if (initRatio != 2)
    {
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        engine = default_random_engine(seed);
        shuffle(dataset.begin(), dataset.end(), engine);

        unsigned seed1 = chrono::system_clock::now().time_since_epoch().count();
        engine = default_random_engine(seed1);
        shuffle(insertDataset.begin(), insertDataset.end(), engine);
    }

    Zipfian zipFind;
    zipFind.InitZipfian(PARAM_ZIPFIAN, dataset.size());
    vector<int> index;
    for (int i = 0; i < dataset.size(); i++)
    {
        int idx = zipFind.GenerateNextIndex();
        index.push_back(idx);
    }
    cout << "shuffle over" << endl;

    if (initRatio == 0.5)
    {
        int end = 50000;

        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            vector<double> rets;
            char key[64] = {0};
            sprintf(key, "%f", dataset[index[i]].first);
            art_search(&t, (const unsigned char *)key, strlen((const char *)key) + 1, rets);
            sprintf(key, "%f", insertDataset[i].first);
            art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1, insertDataset[i].second);
        }
#else
        for (int i = 0; i < end; i++)
        {
            vector<double> rets;
            char key[64] = {0};
            sprintf(key, "%f", dataset[i].first);
            art_search(&t, (const unsigned char *)key, strlen((const char *)key) + 1, rets);
            sprintf(key, "%f", insertDataset[i].first);
            art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1, insertDataset[i].second);
        }
#endif
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            vector<double> rets;
            char key[64] = {0};
            sprintf(key, "%f", dataset[index[i]].first);
            sprintf(key, "%f", insertDataset[i].first);
        }
#else
        for (int i = 0; i < end; i++)
        {
            vector<double> rets;
            char key[64] = {0};
            sprintf(key, "%f", dataset[i].first);
            sprintf(key, "%f", insertDataset[i].first);
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
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[index[findCnt]].first);
                art_search(&t, (const unsigned char *)key, strlen((const char *)key) + 1, rets);
                findCnt++;
            }
            char key[64] = {0};
            sprintf(key, "%f", insertDataset[i].first);
            art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1, insertDataset[i].second);
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[findCnt].first);
                art_search(&t, (const unsigned char *)key, strlen((const char *)key) + 1, rets);
                findCnt++;
            }
            char key[64] = {0};
            sprintf(key, "%f", insertDataset[i].first);
            art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1, insertDataset[i].second);
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
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[index[findCnt]].first);
                findCnt++;
            }
            char key[64] = {0};
            sprintf(key, "%f", insertDataset[i].first);
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[findCnt].first);
                findCnt++;
            }
            char key[64] = {0};
            sprintf(key, "%f", insertDataset[i].first);
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
            vector<double> rets;
            char key[64] = {0};
            sprintf(key, "%f", dataset[index[i]].first);
            art_search(&t, (const unsigned char *)key, strlen((const char *)key) + 1, rets);
        }
#else
        for (int i = 0; i < end; i++)
        {
            vector<double> rets;
            char key[64] = {0};
            sprintf(key, "%f", dataset[i].first);
            art_search(&t, (const unsigned char *)key, strlen((const char *)key) + 1, rets);
        }
#endif
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            vector<double> rets;
            char key[64] = {0};
            sprintf(key, "%f", dataset[index[i]].first);
        }
#else
        for (int i = 0; i < end; i++)
        {
            vector<double> rets;
            char key[64] = {0};
            sprintf(key, "%f", dataset[i].first);
        }
#endif
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / float(dataset.size()) * 1000000000 << endl;
        outRes << tmp / float(dataset.size()) * 1000000000 << ",";
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
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[index[findCnt]].first);
                art_search(&t, (const unsigned char *)key, strlen((const char *)key) + 1, rets);
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                char key[64] = {0};
                sprintf(key, "%f", insertDataset[insertCnt].first);
                art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1, insertDataset[insertCnt].second);
                insertCnt++;
            }
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 17 && findCnt < dataset.size(); j++)
            {
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[findCnt].first);
                art_search(&t, (const unsigned char *)key, strlen((const char *)key) + 1, rets);
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                char key[64] = {0};
                sprintf(key, "%f", insertDataset[insertCnt].first);
                art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1, insertDataset[insertCnt].second);
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
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[index[findCnt]].first);
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                char key[64] = {0};
                sprintf(key, "%f", insertDataset[insertCnt].first);
                insertCnt++;
            }
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 17 && findCnt < dataset.size(); j++)
            {
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[findCnt].first);
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                char key[64] = {0};
                sprintf(key, "%f", insertDataset[insertCnt].first);
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

        for (int i = 0; i < dataset.size(); i++)
        {
            int len = min(i + length[i], dataset.size() - 1);
            dataset[i].second = dataset[len].first;
        }
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        engine = default_random_engine(seed);
        shuffle(dataset.begin(), dataset.end(), engine);

        unsigned seed1 = chrono::system_clock::now().time_since_epoch().count();
        engine = default_random_engine(seed1);
        shuffle(insertDataset.begin(), insertDataset.end(), engine);

        chrono::_V2::system_clock::time_point s, e;
        double tmp;
        s = chrono::system_clock::now();
#if ZIPFIAN
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[index[findCnt]].first);
                char rightKey[64] = {0};
                sprintf(rightKey, "%f", dataset[index[findCnt]].second);

                art_range_scan(&t, (const unsigned char *)key, strlen((const char *)key) + 1, (const unsigned char *)rightKey, strlen((const char *)rightKey) + 1, rets, length[index[findCnt]]);
                findCnt++;
            }
            char key[64] = {0};
            sprintf(key, "%f", insertDataset[i].first);
            art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1, insertDataset[i].second);
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[findCnt].first);
                char rightKey[64] = {0};
                sprintf(rightKey, "%f", dataset[findCnt].second);

                art_range_scan(&t, (const unsigned char *)key, strlen((const char *)key) + 1, (const unsigned char *)rightKey, strlen((const char *)rightKey) + 1, rets, length[findCnt]);
                findCnt++;
            }
            char key[64] = {0};
            sprintf(key, "%f", insertDataset[i].first);
            art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1, insertDataset[i].second);
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
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[index[findCnt]].first);
                char rightKey[64] = {0};
                sprintf(rightKey, "%f", dataset[index[findCnt]].second);
                findCnt++;
            }
            char key[64] = {0};
            sprintf(key, "%f", insertDataset[i].first);
        }
#else
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                vector<double> rets;
                char key[64] = {0};
                sprintf(key, "%f", dataset[findCnt].first);
                char rightKey[64] = {0};
                sprintf(rightKey, "%f", dataset[findCnt].second);
                findCnt++;
            }
            char key[64] = {0};
            sprintf(key, "%f", insertDataset[i].first);
        }
#endif
        e = chrono::system_clock::now();
        double tmp0 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        tmp -= tmp0;

        cout << "total time:" << tmp / 100000.0 * 1000000000 << endl;
        outRes << tmp / 100000.0 * 1000000000 << ",";
    }

    outRes << endl;
    // std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
    //     return p1.first < p2.first;
    // });
    // std::sort(insertDataset.begin(), insertDataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
    //     return p1.first < p2.first;
    // });
}

#endif // !ART_TREE_BASELINE_H