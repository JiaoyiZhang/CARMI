#ifndef DP_H
#define DP_H

#include "../params.h"
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
#include "../function.h"
#include "../inlineFunction.h"
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

// return {cost, true:inner, false:leaf}
pair<pair<double, double>, bool> dp(bool isLeaf, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    if (findSize == 0)
    {
        auto it = COST.find({findLeft, findSize});
        if (it != COST.end())
        {
            auto cost = it->second;
            auto itStruct = structMap.find({false, {findLeft, findSize}});
            auto stru = itStruct->second;
            return {cost, false};
        }
        else
        {
            double cost = kRate * sizeof(ArrayType) / 1024 / 1024;
            COST.insert({{findLeft, findSize}, {0, cost}});
            ParamStruct leafP;
            leafP.type = 4;
            structMap.insert({{false, {findLeft, findSize}}, leafP});
            return {{0, cost}, false};
        }
    }

    // construct a leaf node
    if (isLeaf)
    {
        auto it = COST.find({findLeft, findSize});
        if (it != COST.end())
        {
            auto cost = it->second;
            auto itStruct = structMap.find({false, {findLeft, findSize}});
            auto stru = itStruct->second;
            return {cost, false};
        }
        vector<pair<double, double>> findData;
        vector<pair<double, double>> insertData;
        for (int l = findLeft; l < findLeft + findSize; l++)
            findData.push_back(findDatapoint[l]);
        for (int l = insertLeft; l < insertLeft + insertSize; l++)
            insertData.push_back(insertDatapoint[l]);

        double OptimalValue = DBL_MAX;
        double OptimalTime = DBL_MAX;
        double OptimalSpace = DBL_MAX;
        ParamStruct optimalStruct;
        double space, time, cost;

        // choose an array node as the leaf node
        time = 0.0;
        space = float(sizeof(ArrayType) + 16 * findSize) / 1024 / 1024;

        auto tmp = ArrayType(kMaxKeyNum);
        tmp.model.Train(findData, findData.size());
        auto error = tmp.UpdateError(findData);
        if (error == 0)
            error = 1;
        for (int i = 0; i < findData.size(); i++)
        {
            auto predict = tmp.model.Predict(findData[i].first);
            auto d = abs(i - predict);
            time += 16.36;
            if (d <= error)
                time += log(error) / log(2) * findData[i].second * 4.11;
            else
                time += log(findData.size()) / log(2) * findData[i].second * 4.11;
        }

        for (int i = 0; i < insertData.size(); i++)
        {
            auto predict = tmp.model.Predict(insertData[i].first);
            auto actual = TestArrayBinarySearch(findData[i].first, findData);
            auto d = abs(actual - predict);
            time += 16.36 + 5.25 * (insertData.size() - actual) * insertData[i].second;
            if (d <= error)
                time += log(error) / log(2) * insertData[i].second * 4.11;
            else
                time += log(findData.size()) / log(2) * insertData[i].second * 4.11;
        }
        time = time / (findData.size() + insertData.size());

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
        float Density[4] = {0.5, 0.7, 0.8, 0.9}; // data/capacity
        for (int i = 0; i < 4; i++)
        {
            time = 0.0;
            auto tmpNode = GappedArrayType(kMaxKeyNum);
            tmpNode.density = Density[i];
            space = float(sizeof(GappedArrayType) + 16.0 / tmpNode.density * findData.size()) / 1024 / 1024;

            tmpNode.model.Train(findData, findData.size());
            auto errorGA = tmpNode.UpdateError(findData);
            if (errorGA == 0)
                errorGA = 1;
            for (int t = 0; t < findData.size(); t++)
            {
                auto predict = tmpNode.model.Predict(findData[t].first);
                auto d = abs(t - predict);
                time += 16.36;
                if (d <= errorGA)
                    time += log(errorGA) / log(2) * findData[t].second * 4.11 * (2 - Density[i]);
                else
                    time += log(findData.size()) / log(2) * findData[t].second * 4.11 * (2 - Density[i]);
            }
            for (int t = 0; t < insertData.size(); t++)
            {
                auto predict = tmpNode.model.Predict(insertData[t].first);
                auto actual = TestGABinarySearch(findData[t].first, findData);
                time += 16.36;
                auto d = abs(actual - predict);
                if (d <= errorGA)
                    time += log(errorGA) / log(2) * insertData[t].second * 4.11 * (2 - Density[i]);
                else
                    time += log(findData.size()) / log(2) * insertData[t].second * 4.11 * (2 - Density[i]);
            }
            time = time / (findData.size() + insertData.size());

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
        vector<pair<double, double>> findData;
        vector<pair<double, double>> insertData;
        for (int l = findLeft; l < findLeft + findSize; l++)
            findData.push_back(findDatapoint[l]);
        for (int l = insertLeft; l < insertLeft + insertSize; l++)
            insertData.push_back(insertDatapoint[l]);
        for (int c = 16; c < findData.size(); c *= 2)
        {
            if (512 * c < findData.size())
                continue;
            for (int type = 0; type < 4; type++)
            {
                switch (type)
                {
                case 0:
                {
                    space = float(4 * c + sizeof(LRType)) / 1024 / 1024; // MB
                    double time = 8.1624;                                // ns
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = LRType(c);
                    node.model.Train(findData, c);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = node.model.Predict(findData[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = node.model.Predict(insertData[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
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
                        time += (res.first.first / c);
                        RootCost += res.first.second;      // space*kRate
                        RootCost += (res.first.first / c); // time
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
                    space = float(4 * c + 192 + sizeof(NNType)) / 1024 / 1024; // MB
                    double time = 20.2689;                                     // ns
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = NNType(c);
                    node.model.Train(findData, c);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = node.model.Predict(findData[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = node.model.Predict(insertData[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
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
                        time += (res.first.first / c);
                        RootCost += res.first.second;      // space*kRate
                        RootCost += (res.first.first / c); // time
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
                    space = float(5 * c + sizeof(HisType)) / 1024 / 1024; // MB
                    double time = 19.6543;
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = HisType(c);
                    node.model.Train(findData, c);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = node.model.Predict(findData[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second = subFindData[p].second + 1;
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = node.model.Predict(insertData[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second = subInsertData[p].second + 1;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
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
                        time += (res.first.first / c);
                        RootCost += res.first.second;      // space*kRate
                        RootCost += (res.first.first / c); // time
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
                    space = float(12 * c + sizeof(BSType)) / 1024 / 1024;
                    double time = 4 * log(c) / log(2);
                    double RootCost = time + kRate * space;
                    space *= kRate;
                    if (RootCost > OptimalValue)
                        break;

                    auto node = BSType(c);
                    node.model.Train(findData, c);

                    // divide the key and query
                    vector<pair<int, int>> subFindData(c, {-1, 0});   // {left, size}
                    vector<pair<int, int>> subInsertData(c, {-1, 0}); // {left, size}
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = node.model.Predict(findData[i].first);
                        if (subFindData[p].first == -1)
                            subFindData[p].first = i;
                        subFindData[p].second++;
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = node.model.Predict(insertData[i].first);
                        if (subInsertData[p].first == -1)
                            subInsertData[p].first = i;
                        subInsertData[p].second++;
                    }

                    vector<pair<bool, pair<int, int>>> tmpChild;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, bool> res;
                        if (subFindData[i].second + subInsertData[i].second > 4096)
                            res = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
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
                        time += (res.first.first / c);
                        RootCost += res.first.second;      // space*kRate
                        RootCost += (res.first.first / c); // time
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
        structMap.insert({{true, {findLeft, findSize}}, optimalStruct});
        return {{OptimalTime, OptimalSpace}, true};
    }
}

#endif // !DP_H