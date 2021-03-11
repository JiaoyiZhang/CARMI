#ifndef GREEDY_CHILD_H
#define GREEDY_CHILD_H

#include "../../params.h"
#include "structures.h"
#include "dp_inner.h"
#include "../carmi.h"
#include "minor_function.h"

#include <float.h>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

template <typename TYPE>
void CARMI::CheckGreedy(int c, int type, double pi, int frequency, double time, const DataRange &range, ParamStruct *optimalStruct, NodeCost *optimalCost)
{
    vector<DataRange> perSize(c, emptyRange);
    double space = 64.0 * c / 1024 / 1024;

    TYPE node = TYPE();
    node.SetChildNumber(c);
    Train(&node, range.left, range.size);

    NodePartition<TYPE>(node, range, initDataset, &perSize);
    double entropy = CalculateEntropy(range.size, c, perSize);
    double cost = (time + float(kRate * space) / pi) / entropy;

    if (cost <= optimalCost->cost)
    {
        optimalStruct->type = type;
        optimalStruct->childNum = c;
        *optimalCost = {time * frequency / querySize, kRate * space, cost, true};
    }
}
NodeCost CARMI::GreedyAlgorithm(const IndexPair &dataRange)
{
    NodeCost nodeCost = {0, 0, 0, false};
    if (dataRange.initRange.size == 0 && dataRange.findRange.size == 0)
        return nodeCost;

    NodeCost optimalCost = NodeCost{DBL_MAX, DBL_MAX, DBL_MAX, true};
    double pi = float(dataRange.findRange.size + dataRange.insertRange.size) / querySize;
    ParamStruct optimalStruct = {0, 32, 2, vector<MapKey>()};
    int frequency = 0;
    int findEnd = dataRange.findRange.left + dataRange.findRange.size;
    for (int l = dataRange.findRange.left; l < findEnd; l++)
        frequency += findQuery[l].second;
    int insertEnd = dataRange.insertRange.left + dataRange.insertRange.size;
    for (int l = dataRange.insertRange.left; l < insertEnd; l++)
        frequency += insertQuery[l].second;
    int tmpEnd = dataRange.initRange.size / 2;
    DataRange singleRange(dataRange.initRange.left, dataRange.initRange.size);
    for (int c = 2; c < tmpEnd; c *= 2)
    {
#ifdef DEBUG
        if (c * 512 < dataRange.initRange.size)
            continue;
#endif // DEBUG
        CheckGreedy<LRModel>(c, 0, pi, frequency, LRInnerTime, dataRange.initRange, &optimalStruct, &optimalCost);
        CheckGreedy<LRModel>(c, 1, pi, frequency, PLRInnerTime, dataRange.initRange, &optimalStruct, &optimalCost);
        CheckGreedy<HisModel>(c, 2, pi, frequency, HisInnerTime, dataRange.initRange, &optimalStruct, &optimalCost);
        CheckGreedy<BSModel>(c, 3, pi, frequency, BSInnerTime, dataRange.initRange, &optimalStruct, &optimalCost);
    }

    // construct child
    SubDataset subDataset = SubDataset(optimalStruct.childNum);
    switch (optimalStruct.type)
    {
    case 0:
    {
        InnerDivideAll<LRModel>(optimalStruct.childNum, dataRange, &subDataset);
        break;
    }
    case 1:
    {
        InnerDivideAll<PLRModel>(optimalStruct.childNum, dataRange, &subDataset);
        break;
    }
    case 2:
    {
        InnerDivideAll<HisModel>(optimalStruct.childNum, dataRange, &subDataset);
        break;
    }
    case 3:
    {
        InnerDivideAll<BSModel>(optimalStruct.childNum, dataRange, &subDataset);
        break;
    }
    }

    vector<MapKey> tmpChild;
    for (int i = 0; i < optimalStruct.childNum; i++)
    {
        NodeCost res = {0, 0, 0, true};
        IndexPair range(subDataset.subInit[i], subDataset.subFind[i], subDataset.subInsert[i]);
        if (subDataset.subInit[i].size + subDataset.subInsert[i].size > kAlgorithmThreshold)
            res = GreedyAlgorithm(range);
        else
            res = dp(range);

        MapKey key = {res.isInnerNode, {subDataset.subInit[i].size, subDataset.subInsert[i].size}};
        tmpChild.push_back(key);
    }

    MapKey nodeKey = {true, {dataRange.initRange.left, dataRange.initRange.size}};
    optimalStruct.child = tmpChild;
    if (optimalCost.time < DBL_MAX)
        structMap.insert({nodeKey, optimalStruct});
    nodeCost = {optimalCost.time, optimalCost.space, optimalCost.cost, true};
    return nodeCost;
}
#endif // !GREEDY_H