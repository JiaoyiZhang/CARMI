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

// return {cost, true:inner, false:leaf}
pair<pair<double, double>, bool> GreedyAlgorithm(bool isLeaf, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
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
        space = 16.0 * findSize / 1024 / 1024;

        auto tmp = ArrayType(kMaxKeyNum);
        tmp.Train(findData);
        auto error = tmp.UpdateError(findData);
        if (error == 0)
            error = 1;
        for (int i = 0; i < findData.size(); i++)
        {
            auto predict = tmp.Predict(findData[i].first);
            auto d = abs(i - predict);
            time += 74.6245 * findData[i].second; // due to shuffle
            if (d <= error)
                time += log(error) / log(2) * findData[i].second * 8.23;
            else
                time += log(findData.size()) / log(2) * findData[i].second * 8.23;
        }

        for (int i = 0; i < insertData.size(); i++)
        {
            auto predict = tmp.Predict(insertData[i].first);
            auto actual = TestArrayBinarySearch(findData[i].first, findData);
            auto d = abs(actual - predict);
            time += (74.6245 + 28.25 * (insertData.size() - actual + 1)) * insertData[i].second;
            if (d <= error)
                time += log(error) / log(2) * insertData[i].second * 8.23;
            else
                time += log(insertData.size()) / log(2) * insertData[i].second * 8.23;
        }
        double entropy = log(findData.size()) / log(2);

        cost = (time + space * kRate / findData.size()) / entropy; // ns + MB * kRate
        if (cost <= OptimalValue)
        {
            OptimalValue = cost;
            optimalStruct.type = 4;
            optimalStruct.density = 2;
            OptimalSpace = space * kRate / findData.size() / entropy;
            OptimalTime = time / entropy;
        }

        // choose a gapped array node as the leaf node
        float Density[4] = {0.5, 0.7, 0.8, 0.9}; // data/capacity
        for (int i = 0; i < 4; i++)
        {
            time = 0.0;
            auto tmpNode = GappedArrayType(kMaxKeyNum);
            tmpNode.density = Density[i];
            space = 16.0 / tmpNode.density * findData.size() / 1024 / 1024;

            tmpNode.Train(findData);
            auto errorGA = tmpNode.UpdateError(findData);
            if (errorGA == 0)
                errorGA = 1;
            for (int t = 0; t < findData.size(); t++)
            {
                auto predict = tmpNode.Predict(findData[t].first);
                auto d = abs(t - predict);
                time += 74.6245 * findData[t].second; // due to shuffle
                if (d <= errorGA)
                    time += log(errorGA) / log(2) * findData[t].second * 8.23 * (2 - Density[i]);
                else
                    time += log(findData.size()) / log(2) * findData[t].second * 8.23 * (2 - Density[i]);
            }
            for (int t = 0; t < insertData.size(); t++)
            {
                auto predict = tmpNode.Predict(insertData[t].first);
                auto actual = TestGABinarySearch(findData[t].first, findData);
                time += 74.6245 * insertData[t].second; // due to shuffle
                auto d = abs(actual - predict);
                if (d <= errorGA)
                    time += log(errorGA) / log(2) * insertData[t].second * 8.23 * (2 - Density[i]);
                else
                    time += log(insertData.size()) / log(2) * insertData[t].second * 8.23 * (2 - Density[i]);
            }
            double entropy = log(findData.size()) / log(2);

            cost = (time + space * kRate / findData.size()) / entropy; // ns + MB * kRate
            if (cost <= OptimalValue)
            {
                OptimalValue = cost;
                optimalStruct.type = 5;
                optimalStruct.density = Density[i];
                OptimalSpace = space * kRate / findData.size() / entropy;
                OptimalTime = time / entropy;
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
                vector<int> perSize(c, 0);
                for (int i = 0; i < c; i++)
                    perSize.push_back(0);

                switch (type)
                {
                case 0:
                {
                    time = 12.2345;
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
                    time = 39.1523;
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
                    time = 38.5235;
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
                    time = 8.23 * log(c) / log(2);
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
            if (subFindData[i].second + subInsertData[i].second > 4096)
                res = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
            {
                auto res1 = GreedyAlgorithm(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                auto res0 = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                    res = res1;
                else
                    res = res0;
            }
            else
                res = GreedyAlgorithm(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
            tmpChild.push_back({res.second, {subFindData[i].first, subFindData[i].second}});
            OptimalValue += res.first.first + res.first.second;
            OptimalSpace += res.first.second;
            OptimalTime += res.first.first;
        }

        optimalStruct.child = tmpChild;
        structMap.insert({{true, {findLeft, findSize}}, optimalStruct});
        return {{OptimalTime, OptimalSpace}, true};
    }
}
#endif // !GREEDY_H