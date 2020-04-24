#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "./trainModel/lr.h"
#include "./trainModel/nn.h"

#include "./innerNode/inner_node.h"
#include "./innerNode/nn_node.h"
#include "./innerNode/lr_node.h"
#include "./innerNode/binary_search.h"
#include "./innerNode/division_node.h"

#include "./leafNode/leaf_node.h"
#include "./leafNode/array.h"
#include "./leafNode/gapped_array.h"

#include "./dataset/lognormal_distribution.h"
#include "./dataset/uniform_distribution.h"

#include <algorithm>
#include <iostream>
#include <windows.h>
#include <vector>
using namespace std;

void reconstruction(const vector<pair<double, double>> &data, const vector<pair<int, int>> &cnt)
{
    cout << endl;
    cout << "-------------------------------" << endl;
    cout << "Start reconstruction!" << endl;

    int datasetSize = data.size();
    vector<pair<double, double>> dataset = data;
    btree::btree_map<double, pair<int, int>> cntTree;
    for (int i = 0; i < cnt.size(); i++)
        cntTree.insert({dataset[i].first, cnt[i]}); // <key, <read, write>>

    // Find the minimum childNum
    long double minCost = 1e100;
    int minNum = 0;
    int maxChildNum = dataset.size() / 50;
    for (int i = 2; i < maxChildNum; i++)
    {
        cout << "calculate childNum :" << i << endl;
        // int threshold = float(dataset.size() * 1.2) / i;
        // int capacity = threshold * 0.8;
        int threshold = 1000;
        int capacity = 800;
        cout << "threhold is:" << threshold << "\tcapacity:" << capacity << endl;
        long double tmpCost = LRNode<ArrayNode>::GetCost(cntTree, i, dataset, capacity, threshold);
        cout << "tmpCost is: " << tmpCost << "    minCost: " << minCost << "    minNum: " << minNum << endl;
        if (tmpCost < minCost)
        {
            minCost = tmpCost;
            minNum = i;
        }
    }
    cout << "find the minimum childNum: " << minNum << endl;

    // Rebuild the tree according to the calculated most suitable childNum
    int threshold = float(dataset.size() * 1.4) / minNum;
    int capacity = threshold * 0.8;
    LRNode<ArrayNode> *root = new LRNode<ArrayNode>(threshold, minNum, capacity);
    cout << "Rebuild root over!" << endl;

    //test
    LARGE_INTEGER s, e, c;
    QueryPerformanceCounter(&s);
    QueryPerformanceFrequency(&c);
    for (int i = 0; i < dataset.size(); i++)
    {
        root->Find(dataset[i].first);
    }
    QueryPerformanceCounter(&e);
    cout << "Find time:" << (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart / (float)dataset.size() << endl;
}

#endif