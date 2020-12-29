#ifndef ART_TREE_BASELINE_H
#define ART_TREE_BASELINE_H

#include "../../art_tree/art.h"
#include "../../art_tree/art.cpp"
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>
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
    for (int i = 0; i < dataset.size(); i++)
    {
        auto key = (const unsigned char *)to_string(dataset[i].first).data();
        auto value = (const unsigned char *)to_string(dataset[i].second).data();
        art_insert(&t, key, strlen((const char *)key), (uint64_t)dataset[i].second);
    }
    for (int i = 0; i < insertDataset.size(); i++)
    {
        auto key = (const unsigned char *)to_string(insertDataset[i].first).data();
        auto value = (const unsigned char *)to_string(insertDataset[i].second).data();
        art_insert(&t, key, strlen((const char *)key), (uint64_t)insertDataset[i].second);
    }

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
            auto key = (const unsigned char *)to_string(dataset[i].first).data();
            art_search(&t, key, strlen((const char *)key), rets);
            key = (const unsigned char *)to_string(insertDataset[i].first).data();
            art_insert(&t, key, strlen((const char *)key), (uint64_t)insertDataset[i].second);
        }
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            auto key = (const unsigned char *)to_string(dataset[i].first).data();
            key = (const unsigned char *)to_string(insertDataset[i].first).data();
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
                auto key = (const unsigned char *)to_string(dataset[findCnt].first).data();
                art_search(&t, key, strlen((const char *)key), rets);
                findCnt++;
            }
            auto key = (const unsigned char *)to_string(insertDataset[i].first).data();
            art_insert(&t, key, strlen((const char *)key), (uint64_t)insertDataset[i].second);
        }
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        findCnt = 0;
        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            for (int j = 0; j < 19 && findCnt < dataset.size(); j++)
            {
                auto key = (const unsigned char *)to_string(dataset[findCnt].first).data();
                findCnt++;
            }
            auto key = (const unsigned char *)to_string(insertDataset[i].first).data();
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
            auto key = (const unsigned char *)to_string(dataset[i].first).data();
            art_search(&t, key, strlen((const char *)key), rets);
        }
        e = chrono::system_clock::now();
        tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

        s = chrono::system_clock::now();
        for (int i = 0; i < end; i++)
        {
            auto key = (const unsigned char *)to_string(dataset[i].first).data();
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
                auto key = (const unsigned char *)to_string(dataset[findCnt].first).data();
                art_search(&t, key, strlen((const char *)key), rets);
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                auto key = (const unsigned char *)to_string(insertDataset[insertCnt].first).data();
                art_insert(&t, key, strlen((const char *)key), (uint64_t)insertDataset[insertCnt].second);
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
                auto key = (const unsigned char *)to_string(dataset[findCnt].first).data();
                findCnt++;
            }
            for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++)
            {
                auto key = (const unsigned char *)to_string(insertDataset[insertCnt].first).data();
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