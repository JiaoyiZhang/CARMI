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
void CARMI::CheckGreedy(int c, int type, NodeCost *optimalCost, double time, ParamStruct *optimalStruct, SingleDataRange *range, double pi, int frequency)
{
    SubSingleDataset perSize(c);
    double space = 64.0 * c / 1024 / 1024;

    TYPE node = TYPE();
    node.SetChildNumber(c);
    Train(&node, range->left, range->size);

    NodePartition<TYPE>(&node, &perSize, range, initDataset);

    double entropy = CalculateEntropy(perSize.subSize, range->size, c);
    double cost = (time + float(kRate * space) / pi) / entropy;

    if (cost <= optimalCost->cost)
    {
        optimalStruct->type = type;
        optimalStruct->childNum = c;
        *optimalCost = {time * frequency / querySize, kRate * space, cost, true};
    }
}
NodeCost CARMI::GreedyAlgorithm(DataRange *dataRange)
{
    NodeCost nodeCost = {0, 0, 0, false};
    if (dataRange->initRange.size == 0 && dataRange->findRange.size == 0)
    {
        return nodeCost;
    }

    NodeCost *optimalCost = new NodeCost{DBL_MAX, DBL_MAX, DBL_MAX, true};
    double pi = float(dataRange->findRange.size + dataRange->insertRange.size) / querySize;
    ParamStruct optimalStruct = {0, 32, 2, vector<MapKey>()};
    int frequency = 0;
    int findEnd = dataRange->findRange.left + dataRange->findRange.size;
    for (int l = dataRange->findRange.left; l < findEnd; l++)
        frequency += findQuery[l].second;
    int insertEnd = dataRange->insertRange.left + dataRange->insertRange.size;
    for (int l = dataRange->insertRange.left; l < insertEnd; l++)
        frequency += insertQuery[l].second;
    int tmpEnd = dataRange->initRange.size / 2;
    SingleDataRange singleRange(dataRange->initRange.left, dataRange->initRange.size);
    for (int c = 2; c < tmpEnd; c *= 2)
    {
#ifdef DEBUG
        if (c * 512 < dataRange->initRange.size)
            continue;
#endif // DEBUG
        CheckGreedy<LRModel>(c, 0, optimalCost, LRInnerTime, &optimalStruct, &(dataRange->initRange), pi, frequency);
        CheckGreedy<PLRModel>(c, 1, optimalCost, PLRInnerTime, &optimalStruct, &(dataRange->initRange), pi, frequency);
        CheckGreedy<HisModel>(c, 2, optimalCost, HisInnerTime, &optimalStruct, &(dataRange->initRange), pi, frequency);
        CheckGreedy<BSModel>(c, 3, optimalCost, BSInnerTime, &optimalStruct, &(dataRange->initRange), pi, frequency);
    }

    // construct child
    SubDataset *subDataset = new SubDataset(optimalStruct.childNum);
    switch (optimalStruct.type)
    {
    case 0:
    {
        InnerDivideAll<LRModel>(optimalStruct.childNum, dataRange, subDataset);
        break;
    }
    case 1:
    {
        InnerDivideAll<PLRModel>(optimalStruct.childNum, dataRange, subDataset);
        break;
    }
    case 2:
    {
        InnerDivideAll<HisModel>(optimalStruct.childNum, dataRange, subDataset);
        break;
    }
    case 3:
    {
        InnerDivideAll<BSModel>(optimalStruct.childNum, dataRange, subDataset);
        break;
    }
    }

    vector<MapKey> tmpChild;
    for (int i = 0; i < optimalStruct.childNum; i++)
    {
        NodeCost res = {0, 0, 0, true};
        DataRange *range = new DataRange(subDataset->subInit->subLeft[i], subDataset->subInit->subSize[i], subDataset->subFind->subLeft[i], subDataset->subFind->subSize[i], subDataset->subInsert->subLeft[i], subDataset->subInsert->subSize[i]);
        if (subDataset->subInit->subSize[i] + subDataset->subInsert->subSize[i] > kAlgorithmThreshold)
            res = GreedyAlgorithm(range);
        else
            res = dp(range);

        MapKey key = {res.isInnerNode, {subDataset->subInit->subLeft[i], subDataset->subInit->subSize[i]}};
        tmpChild.push_back(key);
    }

    MapKey nodeKey = {true, {dataRange->initRange.left, dataRange->initRange.size}};
    optimalStruct.child = tmpChild;
    if (optimalCost->time < DBL_MAX)
        structMap.insert({nodeKey, optimalStruct});
    nodeCost = {optimalCost->time, optimalCost->space, optimalCost->cost, true};
    return nodeCost;
}
#endif // !GREEDY_H