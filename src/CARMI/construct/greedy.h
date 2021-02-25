#ifndef GREEDY_CHILD_H
#define GREEDY_CHILD_H

#include "../../params.h"
#include "params_struct.h"
#include "choose_root.h"
#include "dp_inner.h"
#include "../carmi.h"

#include <float.h>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

template <typename TYPE>
void CARMI::InnerAllocSize(vector<int> &perSize, const int c, const int initLeft, const int initSize)
{
    TYPE root = TYPE();
    root.SetChildNumber(c);
    Train(&root, initLeft, initSize);
    int end = initLeft + initSize;
    for (int i = initLeft; i < end; i++)
    {
        int p = root.Predict(initDataset[i].first);
        perSize[p]++;
    }
}

NodeCost CARMI::GreedyAlgorithm(const int initLeft, const int initSize, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    NodeCost nodeCost;
    if (initSize == 0 && findSize == 0)
    {
        nodeCost.space = 0;
        nodeCost.time = 0;
        nodeCost.isInnerNode = false;
        return nodeCost;
    }

    double OptimalValue = DBL_MAX;
    double OptimalTime = DBL_MAX;
    double OptimalSpace = DBL_MAX;
    double time, space;
    double pi = float(initSize + insertSize) / querySize;
    ParamStruct optimalStruct = {0, 32, 2, vector<pair<bool, pair<int, int>>>()};
    int frequency = 0;
    for (int l = findLeft; l < findLeft + findSize; l++)
        frequency += findQuery[l].second;
    for (int l = insertLeft; l < insertLeft + insertSize; l++)
        frequency += insertQuery[l].second;
    int tmpEnd = initSize / 2;
    for (int c = 16; c < tmpEnd; c *= 2)
    {
        if (512 * c < initSize)
            continue;
        for (int type = 0; type < 4; type++)
        {
            space = 64.0 * c / 1024 / 1024;

            if (type == 1)
                continue;
            vector<int> perSize(c, 0);

            switch (type)
            {
            case 0:
            {
                time = 92.4801;
                InnerAllocSize<LRModel>(perSize, c, initLeft, initSize);
                break;
            }
            case 1:
            {
                time = 97.1858;
                InnerAllocSize<PLRModel>(perSize, c, initLeft, initSize);
                break;
            }
            case 2:
            {
                if (c > 160)
                    break;
                time = 109.8874;
                InnerAllocSize<HisModel>(perSize, c, initLeft, initSize);
                break;
            }
            case 3:
            {
                if (c > 20)
                    break;
                time = 114.371;
                InnerAllocSize<BSModel>(perSize, c, initLeft, initSize);
                break;
            }
            }

            long double entropy = 0.0;
            for (int i = 0; i < c; i++)
            {
                auto p = float(perSize[i]) / initSize;
                if (p != 0)
                    entropy += p * (-log(p) / log(2));
            }
            // entropy /= (log(c) / log(2));

            double cost = (time + float(kRate * space) / pi) / entropy;
            if (cost <= OptimalValue)
            {
                OptimalValue = cost;
                OptimalSpace = float(kRate * space);
                OptimalTime = time * frequency / querySize;
                optimalStruct.type = type;
                optimalStruct.childNum = c;
            }
        }
    }

    // construct child
    vector<pair<bool, pair<int, int>>> tmpChild;
    int childNum = optimalStruct.childNum;
    vector<pair<int, int>> subInitData(childNum, {-1, 0});   // {left, size}
    vector<pair<int, int>> subFindData(childNum, {-1, 0});   // {left, size}
    vector<pair<int, int>> subInsertData(childNum, {-1, 0}); // {left, size}
    switch (optimalStruct.type)
    {
    case 0:
    {
        auto node = LRModel();
        node.SetChildNumber(childNum);
        Train(&node, initLeft, initSize);
        InnerDivide<LRModel>(&node, childNum, initLeft, initSize, findLeft, findSize, insertLeft, insertSize, subInitData, subFindData, subInsertData, true);
        break;
    }
    case 1:
    {
        auto node = PLRModel();
        node.SetChildNumber(childNum);
        Train(&node, initLeft, initSize);
        InnerDivide<PLRModel>(&node, childNum, initLeft, initSize, findLeft, findSize, insertLeft, insertSize, subInitData, subFindData, subInsertData, true);
        break;
    }
    case 2:
    {
        auto node = HisModel();
        node.SetChildNumber(childNum);
        Train(&node, initLeft, initSize);
        InnerDivide<HisModel>(&node, childNum, initLeft, initSize, findLeft, findSize, insertLeft, insertSize, subInitData, subFindData, subInsertData, true);
        break;
    }
    case 3:
    {
        auto node = BSModel();
        node.SetChildNumber(childNum);
        Train(&node, initLeft, initSize);
        InnerDivide<BSModel>(&node, childNum, initLeft, initSize, findLeft, findSize, insertLeft, insertSize, subInitData, subFindData, subInsertData, true);
        break;
    }
    }

    for (int i = 0; i < childNum; i++)
    {
        NodeCost res;
        if (subInitData[i].second + subInsertData[i].second > kAlgorithmThreshold)
            res = GreedyAlgorithm(subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
        else
            res = dp(subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
        tmpChild.push_back({res.isInnerNode, {subFindData[i].first, subFindData[i].second}});
        OptimalValue += res.space + res.time;
        OptimalSpace += res.space;
        OptimalTime += res.time;
    }

    optimalStruct.child = tmpChild;
    if (OptimalTime < DBL_MAX)
        structMap.insert({{true, {initLeft, initSize}}, optimalStruct});
    nodeCost.time = OptimalTime;
    nodeCost.space = OptimalSpace;
    nodeCost.isInnerNode = true;
    return nodeCost;
}
#endif // !GREEDY_H