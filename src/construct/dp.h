#ifndef DP_H
#define DP_H

#include "../params.h"
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
#include "../func/function.h"
#include "../func/inlineFunction.h"
#include "greedy_construct.h"
#include "params_struct.h"
#include <float.h>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

extern map<pair<int, int>, pair<double, double>> COST;
;
extern map<pair<bool, pair<int, int>>, ParamStruct> structMap;

extern vector<pair<double, double>> findDatapoint;
extern vector<pair<double, double>> insertDatapoint;

extern int totalFrequency;

// return {cost, true:inner, false:leaf}
pair<pair<double, double>, bool> dp(bool isLeaf, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    if (findSize == 0 && insertSize == 0)
    {
        return {{0, 0}, false};
    }

    // construct a leaf node
    if (isLeaf)
    {
        auto it = COST.find({findLeft, findSize});
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

        // calculate the actual space
        int actualSize = kThreshold;
        while (findSize >= actualSize)
            actualSize *= kExpansionScale;

        // actualSize *= 2; // test
        if (actualSize > 4096)
            actualSize = 4096;

        // choose an array node as the leaf node
        time = 0.0;
        space = 16.0 * actualSize / 1024 / 1024;

        auto tmp = ArrayType(actualSize);
        tmp.Train(findLeft, findSize);
        auto error = tmp.UpdateError(findLeft, findSize);
        for (int i = findLeft; i < findLeft + findSize; i++)
        {
            auto predict = tmp.Predict(findDatapoint[i].first);
            auto d = abs(i - predict);
            time += 161.241 * findDatapoint[i].second;
            if (d <= error)
            {
                if (d > 0 && error > 0)
                    time += log(error) / log(2) * findDatapoint[i].second * 10.9438;
                else
                    time += 2.4132;
            }
            else
                time += log(actualSize) / log(2) * findDatapoint[i].second * 10.9438;
        }

        for (int i = insertLeft; i < insertLeft + insertSize; i++)
        {
            auto predict = tmp.Predict(insertDatapoint[i].first);
            auto actual = TestArrayBinarySearch(insertDatapoint[i].first, findLeft, findLeft + findSize);
            auto d = abs(actual - predict);
            time += (161.241 + 6.25 * (insertSize - actual + 1)) * insertDatapoint[i].second;
            if (d <= error)
            {
                if (d > 0 && error > 0)
                    time += log(error) / log(2) * insertDatapoint[i].second * 10.9438;
                else
                    time += 2.4132;
            }
            else
                time += log(actualSize) / log(2) * insertDatapoint[i].second * 10.9438;
        }
        time = time / totalFrequency;

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
        float Density[4] = {0.5, 0.7, 0.8}; // data/capacity
        for (int i = 0; i < 3; i++)
        {
            // calculate the actual space
            int actualSize = kThreshold;
            while ((float(findSize) / float(actualSize) >= Density[i]))
                actualSize = float(actualSize) / Density[i] + 1;
            actualSize *= 2;
            if (actualSize > 4096)
                actualSize = 4096;

            auto tmpNode = GappedArrayType(actualSize);
            tmpNode.density = Density[i];

            time = 0.0;
            space = 16.0 * actualSize / 1024 / 1024;

            tmpNode.Train(findLeft, findSize);
            auto errorGA = tmpNode.UpdateError(findLeft, findSize);
            for (int t = findLeft; t < findLeft + findSize; t++)
            {
                auto predict = tmpNode.Predict(findDatapoint[t].first);
                auto d = abs(t - predict) * (2 - Density[i]);
                time += 16100.241 * findDatapoint[t].second;
                if (d <= errorGA)
                {
                    if (d > 0 && errorGA > 0)
                        time += log(errorGA) / log(2) * findDatapoint[t].second * 10.9438 * (2 - Density[i]);
                    else
                        time += 2.4132 * (2 - Density[i]);
                }
                else
                    time += log(actualSize) / log(2) * findDatapoint[t].second * 10.9438 * (2 - Density[i]);
            }
            for (int t = insertLeft; t < insertLeft + insertSize; t++)
            {
                auto predict = tmpNode.Predict(insertDatapoint[t].first);
                auto actual = TestGABinarySearch(insertDatapoint[t].first, findLeft, findLeft + findSize);
                time += 161.241 * insertDatapoint[t].second; // due to shuffle
                auto d = abs(actual - predict) * (2 - Density[i]);
                if (d <= errorGA)
                {
                    if (d > 0 && errorGA > 0)
                        time += log(errorGA) / log(2) * insertDatapoint[t].second * 10.9438 * (2 - Density[i]);
                    else
                        time += 2.4132 * (2 - Density[i]);
                }
                else
                    time += log(actualSize) / log(2) * insertDatapoint[t].second * 10.9438 * (2 - Density[i]);
            }
            time = time / totalFrequency;

            cost = time + space * kRate; // ns + MB * kRate
            if (cost <= OptimalValue)
            {
                OptimalValue = cost;
                optimalStruct.type = 5;
                optimalStruct.density = Density[i];
                OptimalSpace = space * kRate;
                OptimalTime = time;
            }
        }
        COST.insert({{findLeft, findSize}, {OptimalTime, OptimalSpace}});
        structMap.insert({{false, {findLeft, findSize}}, optimalStruct});
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
            frequency += findDatapoint[l].second;
        }
        for (int l = insertLeft; l < insertLeft + insertSize; l++)
        {
            frequency += insertDatapoint[l].second;
        }
        int tmpEnd = findSize / 2;
        for (int c = 16; c < tmpEnd; c *= 2)
        {
            if (512 * c < findSize)
                continue;
            for (int type = 0; type < 4; type++)
            {
                if (type == 1)
                    continue;

                switch (type)
                {
                case 0:
                {
                    space = 64.0 * c / 1024 / 1024; // MB
                    double time = 92.4801;          // ns
                    time = time * frequency / totalFrequency;
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = LRModel();
                    node.SetChildNumber(c);
                    node.Train(findLeft, findSize);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    int findEnd = findLeft + findSize;
                    for (int i = findLeft; i < findEnd; i++)
                    {
                        int p = node.Predict(findDatapoint[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    int insertEnd = insertLeft + insertSize;
                    for (int i = insertLeft; i < insertEnd; i++)
                    {
                        int p = node.Predict(insertDatapoint[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
                        {
                            auto res1 = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
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
                    time = time * frequency / totalFrequency;
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = NNModel();
                    node.SetChildNumber(c);
                    node.Train(findLeft, findSize);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    int findEnd = findLeft + findSize;
                    for (int i = findLeft; i < findEnd; i++)
                    {
                        int p = node.Predict(findDatapoint[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    int insertEnd = insertLeft + insertSize;
                    for (int i = insertLeft; i < insertEnd; i++)
                    {
                        int p = node.Predict(insertDatapoint[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
                        {
                            auto res1 = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
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
                    time = time * frequency / totalFrequency;
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = HisModel();
                    node.SetChildNumber(c);
                    node.Train(findLeft, findSize);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    int findEnd = findLeft + findSize;
                    for (int i = findLeft; i < findEnd; i++)
                    {
                        int p = node.Predict(findDatapoint[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    int insertEnd = insertLeft + insertSize;
                    for (int i = insertLeft; i < insertEnd; i++)
                    {
                        int p = node.Predict(insertDatapoint[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
                        {
                            auto res1 = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
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
                    time = time * frequency / totalFrequency;
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = BSModel();
                    node.SetChildNumber(c);
                    node.Train(findLeft, findSize);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    int findEnd = findLeft + findSize;
                    for (int i = findLeft; i < findEnd; i++)
                    {
                        int p = node.Predict(findDatapoint[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    int insertEnd = insertLeft + insertSize;
                    for (int i = insertLeft; i < insertEnd; i++)
                    {
                        int p = node.Predict(insertDatapoint[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                        else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
                        {
                            auto res1 = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                            auto res0 = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                            if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
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
        // COST.insert({{findLeft, findSize}, OptimalValue});
        if (OptimalTime < DBL_MAX)
            structMap.insert({{true, {findLeft, findSize}}, optimalStruct});
        return {{OptimalTime, OptimalSpace}, true};
    }
}

#endif // !DP_H