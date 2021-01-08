#ifndef GREEDY_CHILD_H
#define GREEDY_CHILD_H

#include "../params.h"
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
#include "../func/function.h"
#include "../func/inlineFunction.h"
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

extern int initDatasetSize;
extern int totalFrequency;

extern pair<pair<double, double>, bool> dp(bool isLeaf, const int findLeft, const int findSize, const int insertLeft, const int insertSize);

// return {cost, true:inner, false:leaf}
pair<pair<double, double>, bool> GreedyAlgorithm(bool isLeaf, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    if (findSize == 0 && insertSize == 0)
    {
        auto it = COST.find({findLeft, findSize});
        if (it != COST.end())
        {
            auto cost = it->second;
            return {cost, false};
        }
        else
        {
            double cost = 0.0;
            COST.insert({{findLeft, findSize}, {0, cost}});
            ParamStruct leafP;
            leafP.type = 4;
            leafP.density = 0.5;
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

        actualSize *= 2; // test
        if (actualSize > 4096)
            actualSize = 4096;

        // choose an array node as the leaf node
        time = 0.0;
        space = 16.0 * actualSize / 1024 / 1024;

        auto tmp = ArrayType(kThreshold);
        tmp.Train(findLeft, findSize);
        auto error = tmp.UpdateError(findLeft, findSize);
        for (int i = findLeft; i < findLeft + findSize; i++)
        {
            auto predict = tmp.Predict(findDatapoint[i].first);
            auto d = abs(i - predict);
            time += 175.324 * findDatapoint[i].second;
            if (d <= error)
            {
                if (error > 0)
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
            time += (175.324 + 28.25 * (insertSize - actual + 1)) * insertDatapoint[i].second;
            if (d <= error)
                time += log(error) / log(2) * insertDatapoint[i].second * 10.9438;
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
        float Density[4] = {0.5, 0.7, 0.8, 0.9}; // data/capacity
        for (int i = 0; i < 4; i++)
        {
            auto tmpNode = GappedArrayType(kThreshold);
            tmpNode.density = Density[i];
            space = 16.0 / tmpNode.density * findSize / 1024 / 1024;

            // calculate the actual space
            int actualSize = kThreshold;
            while (findSize >= actualSize)
                actualSize *= kExpansionScale;
            while ((float(findSize) / float(actualSize) >= Density[i]))
                actualSize = float(actualSize) / Density[i] + 1;
            actualSize *= 2;
            if (actualSize > 4096)
                actualSize = 4096;

            time = 0.0;
            space = 16.0 / tmpNode.density * actualSize / 1024 / 1024;

            tmpNode.Train(findLeft, findSize);
            auto errorGA = tmpNode.UpdateError(findLeft, findSize);
            if (errorGA == 0)
                errorGA = 1;
            for (int t = findLeft; t < findLeft + findSize; t++)
            {
                auto predict = tmpNode.Predict(findDatapoint[t].first);
                auto d = abs(t - predict);
                time += 175.324 * findDatapoint[t].second; // due to shuffle
                if (d <= errorGA)
                    time += log(errorGA) / log(2) * findDatapoint[t].second * 10.9438 * (2 - Density[i]);
                else
                    time += log(findSize) / log(2) * findDatapoint[t].second * 10.9438 * (2 - Density[i]);
            }
            for (int t = insertLeft; t < insertLeft + insertSize; t++)
            {
                auto predict = tmpNode.Predict(insertDatapoint[t].first);
                auto actual = TestGABinarySearch(insertDatapoint[t].first, findLeft, findLeft + findSize);
                time += 175.324 * insertDatapoint[t].second; // due to shuffle
                auto d = abs(actual - predict);
                if (d <= errorGA)
                    time += log(errorGA) / log(2) * insertDatapoint[t].second * 10.9438 * (2 - Density[i]);
                else
                    time += log(insertSize) / log(2) * insertDatapoint[t].second * 10.9438 * (2 - Density[i]);
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
        double time, space;
        double pi = float(findSize + insertSize) / (findDatapoint.size() + insertDatapoint.size());
        ParamStruct optimalStruct = {0, 32, 2, vector<pair<bool, pair<int, int>>>()};
        vector<pair<double, double>> findData;
        vector<pair<double, double>> insertData;
        int frequency = 0;
        for (int l = findLeft; l < findLeft + findSize; l++)
        {
            findData.push_back(findDatapoint[l]);
            frequency += findDatapoint[l].second;
        }
        for (int l = insertLeft; l < insertLeft + insertSize; l++)
        {
            insertData.push_back(insertDatapoint[l]);
            frequency += insertDatapoint[l].second;
        }
        for (int c = 16; c < findData.size(); c *= 2)
        {
            if (512 * c < findData.size())
                continue;
            for (int type = 0; type < 4; type++)
            {
                if (type == 1)
                    continue;

                vector<int> perSize(c, 0);
                for (int i = 0; i < c; i++)
                    perSize.push_back(0);

                switch (type)
                {
                case 0:
                {
                    time = 92.4801;
                    space = 64.0 * c / 1024 / 1024;
                    auto root = LRModel();
                    root.SetChildNumber(c);
                    root.Train(findData);
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = root.Predict(findData[i].first);
                        perSize[p]++;
                    }
                    break;
                }
                case 1:
                {
                    time = 97.1858;
                    space = 64.0 * c / 1024 / 1024;
                    auto root = NNModel();
                    root.SetChildNumber(c);
                    root.Train(findData);
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = root.Predict(findData[i].first);
                        perSize[p]++;
                    }
                    break;
                }
                case 2:
                {
                    if (c > 160)
                        break;
                    time = 109.8874;
                    space = 64.0 * c / 1024 / 1024;
                    auto root = HisModel();
                    root.SetChildNumber(c);
                    root.Train(findData);
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = root.Predict(findData[i].first);
                        perSize[p]++;
                    }
                    break;
                }
                case 3:
                {
                    if (c > 20)
                        break;
                    time = 114.371;
                    space = 64.0 * c / 1024 / 1024;
                    auto root = BSModel();
                    root.SetChildNumber(c);
                    root.Train(findData);
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = root.Predict(findData[i].first);
                        perSize[p]++;
                    }
                    break;
                }
                }

                long double entropy = 0.0;
                for (int i = 0; i < c; i++)
                {
                    auto p = float(perSize[i]) / findData.size();
                    if (p != 0)
                        entropy += p * (-log(p) / log(2));
                }
                // entropy /= (log(c) / log(2));

                double cost = (time + float(kRate * space) / pi) / entropy;
                if (cost <= OptimalValue)
                {
                    OptimalValue = cost;
                    OptimalSpace = float(kRate * space) / pi / entropy;
                    OptimalTime = time / entropy;
                    optimalStruct.type = type;
                    optimalStruct.childNum = c;
                }
            }
        }

        // construct child
        vector<pair<bool, pair<int, int>>> tmpChild;
        int childNum = optimalStruct.childNum;
        vector<pair<int, int>> subFindData(childNum, {-1, 0});   // {left, size}
        vector<pair<int, int>> subInsertData(childNum, {-1, 0}); // {left, size}
        switch (optimalStruct.type)
        {
        case 0:
        {
            auto node = LRModel();
            node.SetChildNumber(childNum);
            node.Train(findData);

            // divide the key and query
            for (int i = 0; i < findData.size(); i++)
            {
                int p = node.Predict(findData[i].first);
                if (subFindData[p].first == -1)
                    subFindData[p].first = i;
                subFindData[p].second++;
            }
            for (int i = 0; i < insertData.size(); i++)
            {
                int p = node.Predict(insertData[i].first);
                if (subInsertData[p].first == -1)
                    subInsertData[p].first = i;
                subInsertData[p].second++;
            }
            break;
        }
        case 1:
        {
            auto node = NNModel();
            node.SetChildNumber(childNum);
            node.Train(findData);

            // divide the key and query
            for (int i = 0; i < findData.size(); i++)
            {
                int p = node.Predict(findData[i].first);
                if (subFindData[p].first == -1)
                    subFindData[p].first = i;
                subFindData[p].second++;
            }
            for (int i = 0; i < insertData.size(); i++)
            {
                int p = node.Predict(insertData[i].first);
                if (subInsertData[p].first == -1)
                    subInsertData[p].first = i;
                subInsertData[p].second++;
            }
            break;
        }
        case 2:
        {
            auto node = HisModel();
            node.SetChildNumber(childNum);
            node.Train(findData);

            // divide the key and query
            for (int i = 0; i < findData.size(); i++)
            {
                int p = node.Predict(findData[i].first);
                if (subFindData[p].first == -1)
                    subFindData[p].first = i;
                subFindData[p].second++;
            }
            for (int i = 0; i < insertData.size(); i++)
            {
                int p = node.Predict(insertData[i].first);
                if (subInsertData[p].first == -1)
                    subInsertData[p].first = i;
                subInsertData[p].second++;
            }
            break;
        }
        case 3:
        {
            auto node = BSModel();
            node.SetChildNumber(childNum);
            node.Train(findData);

            // divide the key and query
            for (int i = 0; i < findData.size(); i++)
            {
                int p = node.Predict(findData[i].first);
                if (subFindData[p].first == -1)
                    subFindData[p].first = i;
                subFindData[p].second++;
            }
            for (int i = 0; i < insertData.size(); i++)
            {
                int p = node.Predict(insertData[i].first);
                if (subInsertData[p].first == -1)
                    subInsertData[p].first = i;
                subInsertData[p].second++;
            }
            break;
        }
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> res;
            if (subFindData[i].second + subInsertData[i].second > 40960)
                res = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > 4096)
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
            OptimalValue += res.first.first + res.first.second;
            OptimalSpace += res.first.second;
            OptimalTime += res.first.first;
        }

        optimalStruct.child = tmpChild;
        if (OptimalTime < DBL_MAX)
            structMap.insert({{true, {findLeft, findSize}}, optimalStruct});
        return {{OptimalTime, OptimalSpace}, true};
    }
}
#endif // !GREEDY_H