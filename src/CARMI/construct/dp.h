#ifndef DP_H
#define DP_H

#include "../../params.h"
#include <float.h>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

// return {cost, true:inner, false:leaf}
pair<pair<double, double>, bool> CARMI::dp(bool isLeaf, const int initLeft, const int initSize, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    if (initSize == 0 && findSize == 0)
    {
        return {{0, 0}, false};
    }

    // construct a leaf node
    if (isLeaf)
    {
        auto it = COST.find({initLeft, initSize});
        if (it != COST.end())
        {
            auto cost = it->second;
            return {cost, false};
        }

        double OptimalValue = DBL_MAX;
        double OptimalTime = DBL_MAX;
        double OptimalSpace = DBL_MAX;
        ParamStruct optimalStruct;
        double space, time, cost;

        if (kIsYCSB)
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
            // time = time / querySize;

            cost = time + space * kRate; // ns + MB * kRate
            OptimalValue = cost;
            optimalStruct.type = 6;
            optimalStruct.density = 2;
            OptimalSpace = space * kRate;
            OptimalTime = time;
            return {{OptimalTime, OptimalSpace}, false};
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
                time += (log(actualSize) / log(2) * findQuery[i].second * 10.9438) / querySize;
        }

        for (int i = insertLeft; i < insertLeft + insertSize; i++)
        {
            auto predict = tmp.Predict(insertQuery[i].first) + insertLeft;
            auto actual = TestArrayBinarySearch(insertQuery[i].first, findLeft, findLeft + findSize);
            auto d = abs(actual - predict);
            time += ((161.241 + 6.25 * (findSize + findLeft + insertSize - actual + 1)) * insertQuery[i].second) / querySize;
            if (d <= error)
            {
                if (d > 0 && error > 0)
                    time += (log(error) / log(2) * insertQuery[i].second * 10.9438) / querySize;
                else
                    time += 2.4132 / querySize;
            }
            else
                time += (log(actualSize) / log(2) * insertQuery[i].second * 10.9438) / querySize;
        }

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
        for (int i = 0; i < 1; i++)
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
            for (int t = findLeft; t < findLeft + findSize; t++)
            {
                auto predict = tmpNode.Predict(findQuery[t].first) + findLeft;
                auto d = abs(t - predict);
                time += (161.241 * findQuery[t].second) / querySize;
                if (d <= errorGA)
                {
                    if (d > 0 && errorGA > 0)
                        time += (log(errorGA) / log(2) * findQuery[t].second * 10.9438 * (2 - Density[i])) / querySize;
                    else
                        time += 2.4132 / querySize;
                }
                else
                    time += (log(actualSize) / log(2) * findQuery[t].second * 10.9438 * (2 - Density[i])) / querySize;
            }
            for (int t = insertLeft; t < insertLeft + insertSize; t++)
            {
                auto predict = tmpNode.Predict(insertQuery[t].first) + insertLeft;
                auto actual = TestGABinarySearch(insertQuery[t].first, findLeft, findLeft + findSize);
                time += ((161.241 + 6.25 * (Density[i] / (1 - Density[i]) - 1)) * insertQuery[t].second) / querySize; // due to shuffle
                auto d = abs(actual - predict);
                if (d <= errorGA)
                {
                    if (d > 0 && errorGA > 0)
                        time += (log(errorGA) / log(2) * insertQuery[t].second * 10.9438 * (2 - Density[i])) / querySize;
                    else
                        time += 2.4132 / querySize;
                }
                else
                    time += (log(actualSize) / log(2) * insertQuery[t].second * 10.9438 * (2 - Density[i])) / querySize;
            }
            // time = time / querySize;

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
        return {{OptimalTime, OptimalSpace}, false};
    }
    else
    {
        double OptimalValue = DBL_MAX;
        double OptimalTime = DBL_MAX;
        double OptimalSpace = DBL_MAX;
        double space;
        ParamStruct optimalStruct = {0, 32, 2, vector<pair<bool, pair<int, int>>>()};
        int frequency = 0;
        for (int l = findLeft; l < findLeft + findSize; l++)
        {
            frequency += findQuery[l].second;
        }
        for (int l = insertLeft; l < insertLeft + insertSize; l++)
        {
            frequency += insertQuery[l].second;
        }
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
                    space = 64.0 * c / 1024 / 1024; // MB
                    double time = 92.4801;          // ns
                    time = time * frequency / querySize;
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = LRModel();
                    node.SetChildNumber(c);
                    Train(&node, initLeft, initSize);

                    // divide the key and query
                    vector<pair<int, int>> subInitData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    int initEnd = initLeft + initSize;
                    for (int i = initLeft; i < initEnd; i++)
                    {
                        int p = node.Predict(initDataset[i].first);
                        if (initDataset[p].first == -1)
                            initDataset[p].first = i;
                        initDataset[p].second++;
                    }
                    int findEnd = findLeft + findSize;
                    for (int i = findLeft; i < findEnd; i++)
                    {
                        int p = node.Predict(findQuery[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    int insertEnd = insertLeft + insertSize;
                    for (int i = insertLeft; i < insertEnd; i++)
                    {
                        int p = node.Predict(insertQuery[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subInitData[i].second + subFindData[i].second > 4096)
                            res = GreedyAlgorithm(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subInitData[i].second + subFindData[i].second > kMaxKeyNum)
                        {
                            auto res1 = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = dp(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
                        tmpChild.push_back({res.second, {subFindData[i].first, subFindData[i].second}});
                        space += res.first.second;
                        time += res.first.first;
                        RootCost += res.first.second; // space*kRate
                        RootCost += res.first.first;  // time
                    }
                    if (RootCost <= OptimalValue)
                    {
                        OptimalValue = RootCost;
                        optimalStruct.type = 0;
                        optimalStruct.childNum = c;
                        optimalStruct.child = tmpChild;
                        OptimalTime = time;
                        OptimalSpace = space;
                    }
                    break;
                }
                case 1:
                {
                    space = 64.0 * c / 1024 / 1024; // MB
                    double time = 97.1858;          // ns
                    time = time * frequency / querySize;
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = PLRModel();
                    node.SetChildNumber(c);
                    Train(&node, initLeft, initSize);

                    // divide the key and query
                    vector<pair<int, int>> subInitData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    int initEnd = initLeft + initSize;
                    for (int i = initLeft; i < initEnd; i++)
                    {
                        int p = node.Predict(initDataset[i].first);
                        if (initDataset[p].first == -1)
                            initDataset[p].first = i;
                        initDataset[p].second++;
                    }
                    int findEnd = findLeft + findSize;
                    for (int i = findLeft; i < findEnd; i++)
                    {
                        int p = node.Predict(findQuery[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    int insertEnd = insertLeft + insertSize;
                    for (int i = insertLeft; i < insertEnd; i++)
                    {
                        int p = node.Predict(insertQuery[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subInitData[i].second + subFindData[i].second > 4096)
                            res = GreedyAlgorithm(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subInitData[i].second + subFindData[i].second > kMaxKeyNum)
                        {
                            auto res1 = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = dp(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
                        tmpChild.push_back({res.second, {subFindData[i].first, subFindData[i].second}});
                        space += res.first.second;
                        time += res.first.first;
                        RootCost += res.first.second; // space*kRate
                        RootCost += res.first.first;  // time
                    }
                    if (RootCost <= OptimalValue)
                    {
                        OptimalValue = RootCost;
                        optimalStruct.type = 1;
                        optimalStruct.childNum = c;
                        optimalStruct.child = tmpChild;
                        OptimalTime = time;
                        OptimalSpace = space;
                    }
                    break;
                }
                case 2:
                {
                    if (c > 160)
                        break;
                    space = 64.0 * c / 1024 / 1024; // MB
                    double time = 109.8874;
                    time = time * frequency / querySize;
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = HisModel();
                    node.SetChildNumber(c);
                    Train(&node, initLeft, initSize);

                    // divide the key and query
                    vector<pair<int, int>> subInitData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    int initEnd = initLeft + initSize;
                    for (int i = initLeft; i < initEnd; i++)
                    {
                        int p = node.Predict(initDataset[i].first);
                        if (initDataset[p].first == -1)
                            initDataset[p].first = i;
                        initDataset[p].second++;
                    }
                    int findEnd = findLeft + findSize;
                    for (int i = findLeft; i < findEnd; i++)
                    {
                        int p = node.Predict(findQuery[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    int insertEnd = insertLeft + insertSize;
                    for (int i = insertLeft; i < insertEnd; i++)
                    {
                        int p = node.Predict(insertQuery[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subInitData[i].second + subFindData[i].second > 4096)
                            res = GreedyAlgorithm(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subInitData[i].second + subFindData[i].second > kMaxKeyNum)
                        {
                            auto res1 = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = dp(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
                        tmpChild.push_back({res.second, {subFindData[i].first, subFindData[i].second}});
                        space += res.first.second;
                        time += res.first.first;
                        RootCost += res.first.second; // space*kRate
                        RootCost += res.first.first;  // time
                    }
                    if (RootCost <= OptimalValue)
                    {
                        OptimalValue = RootCost;
                        optimalStruct.type = 2;
                        optimalStruct.childNum = c;
                        optimalStruct.child = tmpChild;
                        OptimalTime = time;
                        OptimalSpace = space;
                    }
                    break;
                }
                case 3:
                {
                    if (c > 20)
                        break;
                    space = 64.0 * c / 1024 / 1024; // MB
                    // double time = 10.9438 * log(c) / log(2);
                    double time = 114.371;
                    time = time * frequency / querySize;
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = BSModel();
                    node.SetChildNumber(c);
                    Train(&node, initLeft, initSize);

                    // divide the key and query
                    vector<pair<int, int>> subInitData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    int initEnd = initLeft + initSize;
                    for (int i = initLeft; i < initEnd; i++)
                    {
                        int p = node.Predict(initDataset[i].first);
                        if (initDataset[p].first == -1)
                            initDataset[p].first = i;
                        initDataset[p].second++;
                    }
                    int findEnd = findLeft + findSize;
                    for (int i = findLeft; i < findEnd; i++)
                    {
                        int p = node.Predict(findQuery[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    int insertEnd = insertLeft + insertSize;
                    for (int i = insertLeft; i < insertEnd; i++)
                    {
                        int p = node.Predict(insertQuery[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subInitData[i].second + subFindData[i].second > 4096)
                            res = GreedyAlgorithm(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subInitData[i].second + subFindData[i].second > kMaxKeyNum)
                        {
                            auto res1 = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = dp(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
                        tmpChild.push_back({res.second, {subFindData[i].first, subFindData[i].second}});
                        space += res.first.second;
                        time += res.first.first;
                        RootCost += res.first.second; // space*kRate
                        RootCost += res.first.first;  // time
                    }
                    if (RootCost <= OptimalValue)
                    {
                        OptimalValue = RootCost;
                        optimalStruct.type = 3;
                        optimalStruct.childNum = c;
                        optimalStruct.child = tmpChild;
                        OptimalTime = time;
                        OptimalSpace = space;
                    }
                    break;
                }
                }
            }
        }
        if (OptimalTime < DBL_MAX)
            structMap.insert({{true, {initLeft, initSize}}, optimalStruct});
        return {{OptimalTime, OptimalSpace}, true};
    }
}

#endif // !DP_H