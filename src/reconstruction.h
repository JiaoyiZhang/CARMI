#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "./trainModel/lr.h"
#include "./trainModel/nn.h"

#include "./innerNode/inner_node.h"
#include "./innerNode/nn_node.h"
#include "./innerNode/lr_node.h"
#include "./innerNode/binary_search.h"
#include "./innerNode/histogram_node.h"

#include "./leafNode/leaf_node.h"
#include "./leafNode/array.h"
#include "./leafNode/gapped_array.h"

#include "./dataset/lognormal_distribution.h"
#include "./dataset/uniform_distribution.h"

#include <algorithm>
#include <iostream>
// #include <windows.h>
#include <vector>
using namespace std;

long double CalculateCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, const vector<pair<double, double>> &data)
{
    long double tmpCost = LRNode::GetCost(cntTree, childNum, data);
    return tmpCost;
}

BasicInnerNode reconstruction(const vector<pair<double, double>> &data, const vector<pair<int, int>> &cnt, const vector<pair<double, double>> &initData)
{
    cout << endl;
    cout << "-------------------------------" << endl;
    cout << "Start reconstruction!" << endl;

    int dataSize = data.size();
    btree::btree_map<double, pair<int, int>> cntTree;
    for (int i = 0; i < cnt.size(); i++)
    {
        cntTree.insert({data[i].first, cnt[i]}); // <key, <read, write>>
    }

    // Find the minimum childNum
    long double minCost = 1e100;
    // int minNum = 0;
    int minNum = 10;
    int maxChildNum = 300;
    long double tmpCost;
    timespec t1, t2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 2; i < maxChildNum; i++)
    {
        // cout << "calculate childNum :" << i << endl;
        tmpCost = CalculateCost(cntTree, i, data);
        // cout << "tmpCost is: " << tmpCost << "    minCost: " << minCost << "    minNum: " << minNum << endl;
        if (tmpCost < minCost)
        {
            minCost = tmpCost;
            minNum = i;
        }
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    double time0 = ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0);
    cout << "find the minimum childNum: " << minNum << endl;
    cout << "time:" << time0 << "ms" << endl;

    // Rebuild the tree according to the calculated most suitable childNum
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    BasicInnerNode root = LRNode(minNum);
    root.Rebuild(initData, cntTree);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    double time = ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0);
    cout << "time:" << time << "ms" << endl;
    cout << "Rebuild root over!" << endl;
    return root;
}

#endif