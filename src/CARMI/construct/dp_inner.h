#ifndef DP_INNER_H
#define DP_INNER_H
#include "../carmi.h"
#include <float.h>
using namespace std;
template <typename TYPE>
void CARMI::InnerDivide(TYPE *node, const int c, const int initLeft, const int initSize, const int findLeft, const int findSize, const int insertLeft, const int insertSize, vector<pair<int, int>> &subInitData, vector<pair<int, int>> &subFindData, vector<pair<int, int>> &subInsertData, bool isNeedFind)
{
    int initEnd = initLeft + initSize;
    for (int i = initLeft; i < initEnd; i++)
    {
        int p = node->Predict(initDataset[i].first);
        if (initDataset[p].first == -1)
            initDataset[p].first = i;
        initDataset[p].second++;
    }
    if (isNeedFind)
    {
        int findEnd = findLeft + findSize;
        for (int i = findLeft; i < findEnd; i++)
        {
            int p = node->Predict(findQuery[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
    }
    int insertEnd = insertLeft + insertSize;
    for (int i = insertLeft; i < insertEnd; i++)
    {
        int p = node->Predict(insertQuery[i].first);
        if (subInsertData[p].first == -1)
            subInsertData[p].first = i;
        subInsertData[p].second++;
    }
}

template <typename TYPE>
void CARMI::CalInner(double &OptimalValue, double &OptimalTime, double &OptimalSpace, ParamStruct &optimalStruct, double &space, double &time, const int frequency, const int c, const int type, const int initLeft, const int initSize, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    space = 64.0 * c / 1024 / 1024; // MB
    time = time * frequency / querySize;
    double RootCost = time + kRate * space;
    space *= kRate;
    if (RootCost > OptimalValue)
        return;

    TYPE node = TYPE();
    node.SetChildNumber(c);
    Train(&node, initLeft, initSize);

    // divide the key and query
    vector<pair<int, int>> subInitData(c, {-1, 0});
    vector<pair<int, int>> subFindData(c, {-1, 0});
    vector<pair<int, int>> subInsertData(c, {-1, 0});
    InnerDivide<TYPE>(&node, c, initLeft, initSize, findLeft, findSize, insertLeft, insertSize, subInitData, subFindData, subInsertData, true);

    vector<pair<bool, pair<int, int>>> tmpChild;
    for (int i = 0; i < c; i++)
    {
        NodeCost res;
        if (subInitData[i].second + subInsertData[i].second > 4096)
            res = GreedyAlgorithm(subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
        else
            res = dp(subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
        tmpChild.push_back({res.isInnerNode, {subFindData[i].first, subFindData[i].second}});
        space += res.space;
        time += res.time;
        RootCost += res.space; // space*kRate
        RootCost += res.time;  // time
    }
    if (RootCost <= OptimalValue)
    {
        OptimalValue = RootCost;
        optimalStruct.type = type;
        optimalStruct.childNum = c;
        optimalStruct.child = tmpChild;
        OptimalTime = time;
        OptimalSpace = space;
    }
}

NodeCost CARMI::dpInner(const int initLeft, const int initSize, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    NodeCost nodeCost;
    double OptimalValue = DBL_MAX;
    double OptimalTime = DBL_MAX;
    double OptimalSpace = DBL_MAX;
    double space;
    ParamStruct optimalStruct = {0, 32, 2, vector<pair<bool, pair<int, int>>>()};
    int frequency = 0;
    for (int l = findLeft; l < findLeft + findSize; l++)
        frequency += findQuery[l].second;
    for (int l = insertLeft; l < insertLeft + insertSize; l++)
        frequency += insertQuery[l].second;
    int tmpEnd = findSize / 2;
    for (int c = 16; c < tmpEnd; c *= 2)
    {
        if (512 * c < findSize)
            continue;
        for (int type = 0; type < 4; type++)
        {
            switch (type)
            {
            case 0:
            {
                double time = 92.4801; // ns
                CalInner<LRModel>(OptimalValue, OptimalTime, OptimalSpace, optimalStruct, space, time, frequency, c, 0, initLeft, initSize, findLeft, findSize, insertLeft, insertSize);
                break;
            }
            case 1:
            {
                double time = 97.1858; // ns
                CalInner<PLRModel>(OptimalValue, OptimalTime, OptimalSpace, optimalStruct, space, time, frequency, c, 1, initLeft, initSize, findLeft, findSize, insertLeft, insertSize);
                break;
            }
            case 2:
            {
                if (c > 160)
                    break;
                double time = 109.8874;
                CalInner<HisModel>(OptimalValue, OptimalTime, OptimalSpace, optimalStruct, space, time, frequency, c, 2, initLeft, initSize, findLeft, findSize, insertLeft, insertSize);
                break;
            }
            case 3:
            {
                if (c > 20)
                    break;
                double time = 114.371;
                CalInner<HisModel>(OptimalValue, OptimalTime, OptimalSpace, optimalStruct, space, time, frequency, c, 3, initLeft, initSize, findLeft, findSize, insertLeft, insertSize);
                break;
            }
            }
        }
    }
    if (OptimalTime < DBL_MAX)
        structMap.insert({{true, {initLeft, initSize}}, optimalStruct});
    nodeCost.time = OptimalTime;
    nodeCost.space = OptimalSpace;
    nodeCost.isInnerNode = true;
    return nodeCost;
}

#endif // !DP_INNER_H