#ifndef DP_LEAF_H
#define DP_LEAF_H

#include "../../params.h"
#include "node_cost_struct.h"
#include "../func/inlineFunction.h"
#include <float.h>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

template <typename TYPE>
void CARMI::CalLeafTime(double &time, const int left, const int size, const int querySize, const int actualSize, const int error, bool isFind, bool isArray, TYPE *node, vector<std::pair<double, double>> &query, const double para) const
{
    for (int i = left; i < left + size; i++)
    {
        auto predict = node->Predict(query[i].first) + left;
        auto d = abs(i - predict);
        time += (161.241 * query[i].second) / querySize;
        if (isFind == false)
        {
            int actual;
            if (isArray)
            {
                actual = TestArrayBinarySearch(query[i].first, left, left + size);
                time += ((6.25 * (size + left + size - actual + 1)) * query[i].second) / querySize;
            }
            else
            {
                actual = TestGABinarySearch(query[i].first, left, left + size);
                time += (6.25 * (2 - para) / (para - 1) - 1) * query[i].second / querySize;
            }
            d = abs(actual - predict);
        }
        if (d <= error)
        {
            if (d > 0 && error > 0)
                time += (log(error) / log(2) * query[i].second * 10.9438) * para / querySize;
            else
                time += 2.4132 / querySize;
        }
        else
            time += (log(actualSize) / log(2) * query[i].second * 10.9438 * para) / querySize;
    }
}

NodeCost CARMI::dpLeaf(const int initLeft, const int initSize, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    NodeCost nodeCost;
    auto it = COST.find({initLeft, initSize});
    if (it != COST.end())
    {
        nodeCost.time = it->second.first;
        nodeCost.space = it->second.second;
        nodeCost.isInnerNode = false;
        return nodeCost;
    }

    double OptimalValue = DBL_MAX;
    double OptimalTime = DBL_MAX;
    double OptimalSpace = DBL_MAX;
    ParamStruct optimalStruct;
    double space, time, cost;

    if (kPrimaryIndex)
    {
        time = 0.0;
        space = 0.0;

        auto tmp = YCSBLeaf();
        Train(&tmp, initLeft, initSize);
        auto error = UpdateError(&tmp, initLeft, initSize);
        for (int i = findLeft; i < findLeft + findSize; i++)
        {
            auto predict = tmp.Predict(findQuery[i].first) + findLeft;
            auto d = abs(i - predict);
            time += (161.241 * findQuery[i].second) / querySize;
            if (d <= error)
            {
                if (d > 0 && error > 0)
                    time += (log(error) / log(2) * findQuery[i].second * 10.9438) / querySize;
                else
                    time += 2.4132 / querySize;
            }
            else
                time += (log(initSize) / log(2) * findQuery[i].second * 10.9438) / querySize;
        }

        for (int i = insertLeft; i < insertLeft + insertSize; i++)
        {
            auto predict = tmp.Predict(insertQuery[i].first) + insertLeft;
            auto actual = TestArrayBinarySearch(insertQuery[i].first, findLeft, findLeft + findSize);
            auto d = abs(actual - predict);
            time += ((161.241 + 6.25) * insertQuery[i].second) / querySize; // due to shuffle
        }

        cost = time + space * kRate; // ns + MB * kRate
        OptimalValue = cost;
        optimalStruct.type = 6;
        optimalStruct.density = 2;
        OptimalSpace = space * kRate;
        OptimalTime = time;

        nodeCost.time = time;
        nodeCost.space = space * kRate;
        nodeCost.isInnerNode = false;
        return nodeCost;
    }

    // calculate the actual space
    int actualSize = kThreshold;
    while (initSize >= actualSize)
        actualSize *= kExpansionScale;

    // actualSize *= 2; // test
    if (actualSize > 4096)
        actualSize = 4096;

    // choose an array node as the leaf node
    time = 0.0;
    space = 16.0 * pow(2, log(actualSize) / log(2) + 1) / 1024 / 1024;

    auto tmp = ArrayType(actualSize);
    Train(&tmp, initLeft, initSize);
    auto error = UpdateError(&tmp, initLeft, initSize);
    CalLeafTime<ArrayType>(time, initLeft, initSize, querySize, actualSize, error, true, true, &tmp, findQuery, 1);
    CalLeafTime<ArrayType>(time, insertLeft, insertSize, querySize, actualSize, error, false, true, &tmp, insertQuery, 1);

    cost = time + space * kRate; // ns + MB * kRate
    if (cost <= OptimalValue)
    {
        OptimalValue = cost;
        optimalStruct.type = 4;
        optimalStruct.density = 2;
        OptimalSpace = space * kRate;
        OptimalTime = time;
    }

    // choose a gapped array node as the leaf node
    float Density[3] = {0.5, 0.7, 0.8}; // data/capacity
    for (int i = 0; i < 3; i++)
    {
        // calculate the actual space
        int actualSize = kThreshold;
        while ((float(initSize) / float(actualSize) >= Density[i]))
            actualSize = float(actualSize) / Density[i] + 1;
        if (actualSize > 4096)
            actualSize = 4096;

        auto tmpNode = GappedArrayType(actualSize);
        tmpNode.density = Density[i];

        time = 0.0;
        space = 16.0 * pow(2, log(actualSize) / log(2) + 1) / 1024 / 1024;

        Train(&tmpNode, initLeft, initSize);
        auto errorGA = UpdateError(&tmpNode, initLeft, initSize);
        CalLeafTime<GappedArrayType>(time, initLeft, initSize, querySize, actualSize, errorGA, true, false, &tmpNode, findQuery, 2 - Density[i]);
        CalLeafTime<GappedArrayType>(time, insertLeft, insertSize, querySize, actualSize, errorGA, false, false, &tmpNode, insertQuery, 2 - Density[i]);
        cost = time + space * kRate; // ns + MB * kRate
        if (cost < OptimalValue)
        {
            OptimalValue = cost;
            optimalStruct.type = 5;
            optimalStruct.density = Density[i];
            OptimalSpace = space * kRate;
            OptimalTime = time;
        }
    }
    COST.insert({{initLeft, initSize}, {OptimalTime, OptimalSpace}});
    structMap.insert({{false, {initLeft, initSize}}, optimalStruct});

    nodeCost.time = OptimalTime;
    nodeCost.space = OptimalSpace;
    nodeCost.isInnerNode = false;
    return nodeCost;
}

#endif // !DP_LEAF_H