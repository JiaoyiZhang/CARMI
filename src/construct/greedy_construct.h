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
        return {{0, 0}, false};
    }

    // construct a leaf node
    if (isLeaf == false)
    {
        double OptimalValue = DBL_MAX;
        double OptimalTime = DBL_MAX;
        double OptimalSpace = DBL_MAX;
        double time, space;
        double pi = float(findSize + insertSize) / (findDatapoint.size() + insertDatapoint.size());
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
                    root.Train(findLeft, findSize);
                    int end = findLeft + findSize;
                    for (int i = findLeft; i < end; i++)
                    {
                        int p = root.Predict(findDatapoint[i].first);
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
                    root.Train(findLeft, findSize);
                    int end = findLeft + findSize;
                    for (int i = findLeft; i < end; i++)
                    {
                        int p = root.Predict(findDatapoint[i].first);
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
                    root.Train(findLeft, findSize);
                    int end = findLeft + findSize;
                    for (int i = findLeft; i < end; i++)
                    {
                        int p = root.Predict(findDatapoint[i].first);
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
                    root.Train(findLeft, findSize);
                    int end = findLeft + findSize;
                    for (int i = findLeft; i < end; i++)
                    {
                        int p = root.Predict(findDatapoint[i].first);
                        perSize[p]++;
                    }
                    break;
                }
                }

                long double entropy = 0.0;
                for (int i = 0; i < c; i++)
                {
                    auto p = float(perSize[i]) / findSize;
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
            node.Train(findLeft, findSize);

            // divide the key and query
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
            break;
        }
        case 1:
        {
            auto node = NNModel();
            node.SetChildNumber(childNum);
            node.Train(findLeft, findSize);

            // divide the key and query
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
            break;
        }
        case 2:
        {
            auto node = HisModel();
            node.SetChildNumber(childNum);
            node.Train(findLeft, findSize);

            // divide the key and query
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
            break;
        }
        case 3:
        {
            auto node = BSModel();
            node.SetChildNumber(childNum);
            node.Train(findLeft, findSize);

            // divide the key and query
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