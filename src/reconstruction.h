#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "./trainModel/lr.h"
#include "./trainModel/nn.h"

#include "./innerNode/innerNode.h"
#include "./innerNode/nnNode.h"
#include "./innerNode/lrNode.h"
#include "./innerNode/binarySearch.h"
#include "./innerNode/divisionNode.h"

#include "./leafNode/leafNode.h"
#include "./leafNode/array.h"
#include "./leafNode/gappedArray.h"

#include "./dataset/lognormalDistribution.h"
#include "./dataset/uniformDistribution.h"

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

    // // lognormalDistribution params
    // params firstStageParams(0.001, 100000, 8, 0.0001, 0.666);
    // params secondStageParams(0.001, 100000, 8, 0.0001, 0.666);

    // uniformDistrubution params
    params firstStageParams(0.00001, 500, 8, 0.0001, 0.00001);
    params secondStageParams(0.0000001, 1, 10000, 8, 0.0, 0.0);

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
        long double tmpCost = lrNode<arrayNode>::getCost(cntTree, i, dataset, capacity, threshold);
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
    lrNode<arrayNode> *root = new lrNode<arrayNode>(firstStageParams, secondStageParams, threshold, minNum, capacity);
    cout << "Rebuild root over!" << endl;

    //test
    LARGE_INTEGER s, e, c;
    QueryPerformanceCounter(&s);
    QueryPerformanceFrequency(&c);
    for (int i = 0; i < dataset.size(); i++)
    {
        root->find(dataset[i].first);
    }
    QueryPerformanceCounter(&e);
    cout << "Find time:" << (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart / (float)dataset.size() << endl;
}

#endif