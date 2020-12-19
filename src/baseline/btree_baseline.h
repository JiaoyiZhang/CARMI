#ifndef BTREE_BASELINE_H
#define BTREE_BASELINE_H

#include <iostream>
#include <fstream>
#include <vector>
#include "../../stx_btree/btree_map.h"
using namespace std;

extern ofstream outRes;
extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

void btree_test()
{
    stx::btree_map<double, double> btree;
    for (int l = 0; l < dataset.size(); l++)
        btree.insert(dataset[l]);
    cout << "btree:" << endl;
    // chrono::_V2::system_clock::time_point s, e;
    double tmp;
    // s = chrono::system_clock::now();
    for (int i = 0; i < dataset.size(); i++)
        btree.find(dataset[i].first);
    // e = chrono::system_clock::now();
    // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "Find time:" << tmp / (float)dataset.size() * 1000000000 << "ns" << endl;
    outRes << "btree," << tmp / (float)dataset.size() * 1000000000 << ",";

    // s = chrono::system_clock::now();
    for (int i = 0; i < insertDataset.size(); i++)
        btree.insert(insertDataset[i]);
    // e = chrono::system_clock::now();
    // tmp = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
    cout << "Insert time:" << tmp / (float)insertDataset.size() * 1000000000 << endl;
    outRes << tmp / (float)insertDataset.size() * 1000000000 << "\n";
    cout << endl;
}

#endif // !ART_TREE_BASELINE_H