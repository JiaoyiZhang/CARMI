#ifndef DP_LEAF_H
#define DP_LEAF_H

#include "../../params.h"
#include "structures.h"
#include "../func/inlineFunction.h"
#include <float.h>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

template <typename TYPE>
double CARMI::CalLeafFindTime(TYPE *node, SingleDataRange *range, const int actualSize, const double density) const
{
    double time = 0;
    for (int i = range->left; i < range->left + range->size; i++)
    {
        auto predict = node->Predict(findQuery[i].first) + range->left;
        auto d = abs(i - predict);
        time += (CostBaseTime * findQuery[i].second) / querySize;
        if (d <= node->error)
        {
            time += (log(node->error + 1) / log(2) * findQuery[i].second * CostBSTime) * (2 - density) / querySize;
        }
        else
            time += (log(actualSize) / log(2) * findQuery[i].second * CostBSTime * (2 - density)) / querySize;
    }
    return time;
}

template <typename TYPE>
double CARMI::CalLeafInsertTime(TYPE *node, SingleDataRange *range, SingleDataRange *findRange, const int actualSize, const double density) const
{
    double time = 0;
    for (int i = range->left; i < range->left + range->size; i++)
    {
        int predict = node->Predict(insertQuery[i].first) + findRange->left;
        int d = abs(i - predict);
        time += (CostBaseTime * insertQuery[i].second) / querySize;
        int actual;
        if (density == 1)
        {
            actual = TestBinarySearch(insertQuery[i].first, findRange->left, findRange->left + findRange->size);
            time += CostMoveTime * findRange->size / 2 * insertQuery[i].second / querySize;
        }
        else
        {
            actual = TestBinarySearch(insertQuery[i].first, findRange->left, findRange->left + findRange->size);
            time += CostMoveTime * density / (1 - density) * insertQuery[i].second / querySize;
        }
        d = abs(actual - predict);

        if (d <= node->error)
        {
            time += (log(node->error + 1) / log(2) * insertQuery[i].second * CostBSTime) * (2 - density) / querySize;
        }
        else
            time += (log(actualSize) / log(2) * insertQuery[i].second * CostBSTime * (2 - density)) / querySize;
    }
    return time;
}

NodeCost CARMI::dpLeaf(DataRange *dataRange)
{
    NodeCost nodeCost;
    auto it = COST.find(dataRange->initRange);
    if (it != COST.end())
    {
        nodeCost = it->second;
        nodeCost.isInnerNode = false;
        return nodeCost;
    }

    NodeCost *optimalCost = new NodeCost{DBL_MAX, DBL_MAX, DBL_MAX, false};
    ParamStruct optimalStruct;

    if (kPrimaryIndex)
    {
        nodeCost.time = 0.0;
        nodeCost.space = 0.0;

        auto tmp = YCSBLeaf();
        Train(&tmp, dataRange->initRange.left, dataRange->initRange.size);
        auto error = UpdateError(&tmp, dataRange->initRange.left, dataRange->initRange.size);
        int findEnd = dataRange->findRange.left + dataRange->findRange.size;
        for (int i = dataRange->findRange.left; i < findEnd; i++)
        {
            auto predict = tmp.Predict(findQuery[i].first) + dataRange->findRange.left;
            auto d = abs(i - predict);
            nodeCost.time += (CostBaseTime * findQuery[i].second) / querySize;
            if (d <= error)
            {
                nodeCost.time += (log(error + 1) / log(2) * findQuery[i].second * CostBSTime) / querySize;
            }
            else
                nodeCost.time += (log(dataRange->initRange.size) / log(2) * findQuery[i].second * CostBSTime) / querySize;
        }

        int insertEnd = dataRange->insertRange.left + dataRange->insertRange.size;
        for (int i = dataRange->insertRange.left; i < insertEnd; i++)
        {
            nodeCost.time += ((CostBaseTime + CostMoveTime) * insertQuery[i].second) / querySize;
        }

        nodeCost.cost = nodeCost.time + nodeCost.space * kRate; // ns + MB * kRate
        *optimalCost = {nodeCost.time, nodeCost.space * kRate, nodeCost.cost, false};
        optimalStruct = ParamStruct(6, 0, 2, vector<MapKey>());

        nodeCost.space *= kRate;
        nodeCost.isInnerNode = false;
        return nodeCost;
    }

    int actualSize = kThreshold;
    while (dataRange->initRange.size >= actualSize)
        actualSize *= kExpansionScale;

    if (actualSize > 4096)
        actualSize = 4096;

    // choose an array node as the leaf node
    double time = 0.0;
    double space = 16.0 * pow(2, log(actualSize) / log(2) + 1) / 1024 / 1024;

    auto tmp = ArrayType(actualSize);
    Train(&tmp, dataRange->initRange.left, dataRange->initRange.size);
    auto error = UpdateError(&tmp, dataRange->initRange.left, dataRange->initRange.size);
    CalLeafFindTime<ArrayType>(&tmp, &(dataRange->findRange), actualSize, 1);
    CalLeafInsertTime<ArrayType>(&tmp, &(dataRange->insertRange), &(dataRange->findRange), actualSize, 1);

    double cost = time + space * kRate; // ns + MB * kRate
    if (cost <= optimalCost->cost)
    {
        optimalCost = new NodeCost{time, space * kRate, cost, false};
        optimalStruct = ParamStruct(4, 0, 1, vector<MapKey>());
    }

    // choose a gapped array node as the leaf node
    float Density[3] = {0.5, 0.7, 0.8}; // data/capacity
    for (int i = 0; i < 3; i++)
    {
        // calculate the actual space
        int actualSize = kThreshold;
        while ((float(dataRange->initRange.size) / float(actualSize) >= Density[i]))
            actualSize = float(actualSize) / Density[i] + 1;
        if (actualSize > 4096)
            actualSize = 4096;

        auto tmpNode = GappedArrayType(actualSize);
        tmpNode.density = Density[i];

        time = 0.0;
        space = 16.0 * pow(2, log(actualSize) / log(2) + 1) / 1024 / 1024;

        Train(&tmpNode, dataRange->initRange.left, dataRange->initRange.size);
        auto errorGA = UpdateError(&tmpNode, dataRange->initRange.left, dataRange->initRange.size);
        CalLeafFindTime<GappedArrayType>(&tmpNode, &(dataRange->findRange), actualSize, Density[i]);
        CalLeafInsertTime<GappedArrayType>(&tmpNode, &(dataRange->insertRange), &(dataRange->findRange), actualSize, Density[i]);
        cost = time + space * kRate; // ns + MB * kRate
        if (cost <= optimalCost->cost)
        {
            optimalCost = new NodeCost{time, space * kRate, cost, false};
            optimalStruct = ParamStruct(5, 0, Density[i], vector<MapKey>());
        }
    }

    MapKey key = {false, {dataRange->initRange.left, dataRange->initRange.size}};
    COST.insert({dataRange->initRange, *optimalCost});
    structMap.insert({key, optimalStruct});

    return *optimalCost;
}

#endif // !DP_LEAF_H