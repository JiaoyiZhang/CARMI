#ifndef DP_INNER_H
#define DP_INNER_H
#include "../carmi.h"
#include <float.h>
using namespace std;

template <typename TYPE>
void CARMI::CalInner(NodeCost *optimalCost, ParamStruct *optimalStruct, double time, const int frequency, const int c, const int type, DataRange *dataRange)
{
    double space = 64.0 * c / 1024 / 1024; // MB
    time = time * frequency / querySize;
    double RootCost = time + kRate * space;
    space *= kRate;
    if (RootCost > optimalCost->cost)
        return;

    SubDataset subDataset(c);
    InnerDivideAll<TYPE>(c, dataRange, &subDataset);

    vector<MapKey> tmpChild;
    for (int i = 0; i < c; i++)
    {
        NodeCost res;
        DataRange range(subDataset.subInit->subLeft[i], subDataset.subInit->subSize[i], subDataset.subFind->subLeft[i], subDataset.subFind->subSize[i], subDataset.subInsert->subLeft[i], subDataset.subInsert->subSize[i]);
        if (subDataset.subInit->subSize[i] + subDataset.subInsert->subSize[i] > kAlgorithmThreshold)
            res = GreedyAlgorithm(&range);
        else
            res = dp(&range);

        MapKey key = {res.isInnerNode, {subDataset.subInit->subLeft[i], subDataset.subInit->subSize[i]}};
        tmpChild.push_back(key);
        space += res.space;
        time += res.time;
        RootCost += res.space + res.time;
    }
    if (RootCost <= optimalCost->cost)
    {
        *optimalCost = {time, space, RootCost, true};
        *optimalStruct = ParamStruct(type, c, kDensity, tmpChild);
    }
}

NodeCost CARMI::dpInner(DataRange *dataRange)
{
    NodeCost nodeCost;
    NodeCost *optimalCost = new NodeCost{DBL_MAX, DBL_MAX, DBL_MAX, true};
    double space;
    ParamStruct optimalStruct = {0, 32, 2, vector<MapKey>()};
    int frequency = 0;
    int findEnd = dataRange->findRange.left + dataRange->findRange.size;
    for (int l = dataRange->findRange.left; l < findEnd; l++)
        frequency += findQuery[l].second;
    int insertEnd = dataRange->insertRange.left + dataRange->insertRange.size;
    for (int l = dataRange->insertRange.left; l < insertEnd; l++)
        frequency += insertQuery[l].second;
    int tmpEnd = dataRange->initRange.size / 2;
    for (int c = 2; c < tmpEnd; c *= 2)
    {
#ifdef DEBUG
        if (c * 512 < dataRange->initRange.size)
            continue;
#endif // DEBUG
        CalInner<LRModel>(optimalCost, &optimalStruct, LRInnerTime, frequency, c, 0, dataRange);
        CalInner<PLRModel>(optimalCost, &optimalStruct, PLRInnerTime, frequency, c, 1, dataRange);
        if (c <= 160)
            CalInner<HisModel>(optimalCost, &optimalStruct, HisInnerTime, frequency, c, 2, dataRange);
        if (c <= 20)
            CalInner<HisModel>(optimalCost, &optimalStruct, BSInnerTime, frequency, c, 3, dataRange);
    }
    MapKey key = {true, {dataRange->initRange.left, dataRange->initRange.size}};
    if (optimalCost->time < DBL_MAX)
        structMap.insert({key, optimalStruct});
    nodeCost = {optimalCost->time, optimalCost->space, optimalCost->cost, true};
    delete optimalCost;
    return nodeCost;
}

#endif // !DP_INNER_H