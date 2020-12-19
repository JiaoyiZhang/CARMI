#ifndef ART_TREE_BASELINE_H
#define ART_TREE_BASELINE_H

#include "../../art_tree/art.h"
#include "../../art_tree/art.cpp"
#include <fstream>
using namespace std;

extern ofstream outRes;
extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

void artTree_test()
{
    outRes << "artTree:,";
    art_tree t;
    art_tree_init(&t);
    vector<char *> artKey, artValue, artInsertKey, artInsertValue;
    for (int i = 0; i < dataset.size(); i++)
    {
        auto key = (const unsigned char *)to_string(dataset[i].first).data();
        auto value = (const unsigned char *)to_string(dataset[i].second).data();
        artKey.push_back((char *)key);
        artValue.push_back((char *)value);
        art_insert(&t, key, strlen((const char *)key), (uint64_t)dataset[i].second);
    }
    for (int i = 0; i < insertDataset.size(); i++)
    {
        auto key = (const unsigned char *)to_string(insertDataset[i].first).data();
        auto value = (const unsigned char *)to_string(insertDataset[i].second).data();
        artInsertKey.push_back((char *)key);
        artInsertValue.push_back((char *)value);
        art_insert(&t, key, strlen((const char *)key), (uint64_t)insertDataset[i].second);
    }
    vector<uint64_t> rets;
    // chrono::_V2::system_clock::time_point s, e;
    double tmp, tmp1;
    // s = chrono::system_clock::now();
    for (int i = 0; i < artKey.size(); i++)
    {
        auto key = (const unsigned char *)to_string(dataset[i].first).data();
    }
    // e = chrono::system_clock::now();
    // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

    // s = chrono::system_clock::now();
    for (int i = 0; i < artKey.size(); i++)
    {
        auto key = (const unsigned char *)to_string(dataset[i].first).data();
        art_search(&t, key, strlen((const char *)key), rets);
    }
    // e = chrono::system_clock::now();
    // tmp1 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "art find time: " << tmp1 / (float)dataset.size() * 1000000000 << endl;
    cout << "other time:" << tmp / (float)dataset.size() * 1000000000 << endl;
    cout << "final time:" << (tmp1 - tmp) / (float)dataset.size() * 1000000000 << endl;
    outRes << (tmp1 - tmp) / (float)dataset.size() * 1000000000 << ",";

    // s = chrono::system_clock::now();
    for (int i = 0; i < artInsertKey.size(); i++)
    {
        auto key = (const unsigned char *)to_string(insertDataset[i].first).data();
    }
    // e = chrono::system_clock::now();
    // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

    // s = chrono::system_clock::now();
    for (int i = 0; i < artInsertKey.size(); i++)
    {
        auto key = (const unsigned char *)to_string(insertDataset[i].first).data();
        art_insert(&t, key, strlen((const char *)key), (uint64_t)insertDataset[i].second);
    }
    // e = chrono::system_clock::now();
    // tmp1 = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "art insert time: " << tmp1 / (float)insertDataset.size() * 1000000000 << endl;
    cout << "other time:" << tmp / (float)insertDataset.size() * 1000000000 << endl;
    cout << "final insert time:" << (tmp1 - tmp) / (float)insertDataset.size() * 1000000000 << endl;
    outRes << (tmp1 - tmp) / (float)insertDataset.size() * 1000000000 << "\n";
}

#endif // !ART_TREE_BASELINE_H