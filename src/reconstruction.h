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

long double CalculateCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, const vector<pair<double, double>> &dataset)
{
    long double tmpCost = HistogramNode::GetCost(cntTree, childNum, dataset);
    return tmpCost;
}

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
    long double tmpCost;
    for (int i = 2; i < maxChildNum; i++)
    {
        cout << "calculate childNum :" << i << endl;
        tmpCost = CalculateCost(cntTree, i, dataset);
        cout << "tmpCost is: " << tmpCost << "    minCost: " << minCost << "    minNum: " << minNum << endl;
        if (tmpCost < minCost)
        {
            minCost = tmpCost;
            minNum = i;
        }
    }
    cout << "find the minimum childNum: " << minNum << endl;

    // Rebuild the tree according to the calculated most suitable childNum
    HistogramNode *root = new HistogramNode(minNum);
    cout << "Rebuild root over!" << endl;
}

#endif