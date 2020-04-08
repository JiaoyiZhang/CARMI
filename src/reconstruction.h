#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "./trainModel/lr.h"
#include "./trainModel/nn.h"
#include "./innerNode/adaptiveRMI.h"
#include "./innerNode/staticRMI.h"
#include "./innerNode/scaleModel.h"
#include "./leafNode/gappedNode.h"
#include "./leafNode/normalNode.h"
#include "./dataset/lognormalDistribution.h"
#include "./dataset/uniformDistribution.h"
#include "node.h"

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
    node root;

    int datasetSize = data.size();
    btree::btree_map<double, pair<int, int>> cntTree;
    for (int i = 0; i < cnt.size(); i++)
        cntTree.insert(dataset[i].first, cnt[i]); // <key, <read, write>>
    int totalReadNum = 0, totalWriteNum = 0;
    for (int i = 0; i < datasetSize; i++)
    {
        totalReadNum += cnt[i].first;
        totalWriteNum += cnt[i].second;
    }

    // // lognormalDistribution params
    // params firstStageParams(0.001, 100000, 8, 0.0001, 0.666);
    // params secondStageParams(0.001, 100000, 8, 0.0001, 0.666);

    // uniformDistrubution params
    params firstStageParams(0.00001, 5000, 8, 0.0001, 0.00001);
    params secondStageParams(0.0000001, 1, 10000, 8, 0.0, 0.0);

    // Select the innerNode model
    bool rootIsScale = true;
    if (rootIsScale)
        root = scaleModel<gappedNode<linearRegression>>(secondStageParams, 1000, 100, 800);
    else
        root = adaptiveRMI<gappedNode<linearRegression>, linearRegression>(firstStageParams, secondStageParams, 1000, 12, 800);
    cout << "root init over!" << endl;

    // Find the minimum childNum
    long double minCost = 1e100;
    int minNum = 0;
    for (int i = 2; i < dataset.size(); i++)
    {
        long double tmpCost = root.getCost(cntTree, i);
        if (tmpCost < minCost)
        {
            minCost = tmpCost;
            minNum = i;
        }
    }
    cout << "find the minimum childNum: " << minNum << endl;

    // Rebuild the tree according to the calculated most suitable childNum
    int threshold = float(dataset.size() * 1.2) / minNum;
    int capacity = threshold * 0.8;
    if (rootIsScale)
        root = scaleModel<gappedNode<linearRegression>>(secondStageParams, threshold, minNum, capacity);
    else
        root = adaptiveRMI<gappedNode<linearRegression>, linearRegression>(firstStageParams, secondStageParams, threshold, minNum, capacity);
    cout << "Rebuild root over!" << endl;

    // Adjust the structure of leaf nodes
    root.change(cntTree);
    cout << "Adjust leaf nodes over!" << endl;

    //test
    LARGE_INTEGER s, e, c;
    QueryPerformanceCounter(&s);
    QueryPerformanceFrequency(&c);
    for (int i = 0; i < dataset.size(); i++)
    {
        root.find(dataset[i].first);
    }
    QueryPerformanceCounter(&e);
    cout << "Find time:" << (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart / (float)dataset.size() << endl;
}

#endif