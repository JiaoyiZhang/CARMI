#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include "choose_root.h"
#include "store_node.h"
#include "dp.h"
#include "params_struct.h"
#include "update_leaf.h"
#include "../carmi.h"
#include <vector>
#include <map>
using namespace std;

// main function of construction
// return the type of root
// findDatapoint: the dataset used to initialize the index
// insertDatapoint: the dataset to be inserted into the index
// readCnt: the number of READ corresponding to each key
// writeCnt: the number of WRITE corresponding to each key
inline int CARMI::Construction(const vector<pair<double, double>> &initData, const vector<pair<double, double>> &findData, const vector<pair<double, double>> &insertData)
{
    cout << endl;
    cout << "-------------------------------" << endl;
    cout << "Start construction!" << endl;
    querySize = findData.size() + insertData.size();
    time_t timep;
    time(&timep);
    char tmpTime[64];
    strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cout << "\nTEST time: " << tmpTime << endl;

    if (!kIsYCSB)
        initEntireData(0, initData.size() * 1.1, false);
    initEntireChild(initData.size() * 1.1);
    initDataset = initData;
    findQuery = findData;
    insertQuery = insertData;
    auto res = ChooseRoot(findData);

    COST.clear();
    structMap.clear();
    scanLeaf.clear();

    int childNum = res.second;
    rootType = res.first;
    kInnerNodeID = rootType;
    cout << "Construction of the root node has been completed!" << endl;
    cout << "The optimal value of root is: " << res.first << ",\tthe optimal child number is: " << res.second << endl;
    switch (rootType)
    {
    case 0:
    {
        root.lrRoot = LRType(childNum);
        root.lrRoot.childLeft = allocateChildMemory(childNum);
        root.lrRoot.model.Train(findData, childNum);
        break;
    }
    case 1:
    {
        root.plrRoot = PLRType(childNum);
        root.plrRoot.childLeft = allocateChildMemory(childNum);
        root.plrRoot.model.Train(findData, childNum);
        break;
    }
    case 2:
    {
        root.hisRoot = HisType(childNum);
        root.hisRoot.childLeft = allocateChildMemory(childNum);
        root.hisRoot.model.Train(findData, childNum);
        break;
    }
    case 3:
    {
        root.bsRoot = BSType(childNum);
        root.bsRoot.childLeft = allocateChildMemory(childNum);
        root.bsRoot.model.Train(findData, childNum);
        break;
    }
    }
    double totalCost = 0.0;
    double totalTime = 0.0;
    double totalSpace = 0.0;
    vector<pair<int, int>> subInitData(childNum, {-1, 0});   // {left, size}
    vector<pair<int, int>> subFindData(childNum, {-1, 0});   // {left, size}
    vector<pair<int, int>> subInsertData(childNum, {-1, 0}); // {left, size}

    time(&timep);
    char tmpTime2[64];
    strftime(tmpTime2, sizeof(tmpTime2), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cout << "Root time: " << tmpTime2 << endl;

    COST.insert({{-1, 0}, {0, 0}});
    ParamStruct leafP;
    leafP.type = 4;
    leafP.density = 0.5;
    structMap.insert({{false, {-1, 0}}, leafP});

    switch (rootType)
    {
    case 0:
    {
        totalTime = 12.7013;
        totalSpace += sizeof(LRType);
        for (int i = 0; i < initDataset.size(); i++)
        {
            int p = root.lrRoot.model.Predict(initDataset[i].first);
            if (subInitData[p].first == -1)
                subInitData[p].first = i;
            subInitData[p].second++;
        }
        for (int i = 0; i < findQuery.size(); i++)
        {
            int p = root.lrRoot.model.Predict(findQuery[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertQuery.size(); i++)
        {
            int p = root.lrRoot.model.Predict(insertQuery[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = GreedyAlgorithm(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
            {
                auto res0 = dp(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                auto res1 = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
            }
            int type;
            pair<bool, pair<int, int>> key = {resChild.second, {subInitData[i].first, subInitData[i].second}};
            auto it = structMap.find(key);
            if (it == structMap.end())
            {
                if (kIsYCSB)
                    type = 6;
                else
                    type = 4;
            }
            else
                type = it->second.type;
            storeOptimalNode(type, key, subInitData[i].first, subInitData[i].second, subInsertData[i].first, subInsertData[i].second, i);

            totalCost += resChild.first.first + resChild.first.second;
            totalTime += resChild.first.first;
            totalSpace += resChild.first.second;

            COST.clear();
            structMap.clear();
        }
        break;
    }
    case 1:
    {
        totalTime = 39.6429;
        totalSpace += sizeof(PLRType);
        for (int i = 0; i < initDataset.size(); i++)
        {
            int p = root.lrRoot.model.Predict(initDataset[i].first);
            if (subInitData[p].first == -1)
                subInitData[p].first = i;
            subInitData[p].second++;
        }
        for (int i = 0; i < findQuery.size(); i++)
        {
            int p = root.lrRoot.model.Predict(findQuery[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertQuery.size(); i++)
        {
            int p = root.lrRoot.model.Predict(insertQuery[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = GreedyAlgorithm(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
            {
                auto res0 = dp(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                auto res1 = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
            }
            int type;
            pair<bool, pair<int, int>> key = {resChild.second, {subInitData[i].first, subInitData[i].second}};
            auto it = structMap.find(key);
            if (it == structMap.end())
            {
                if (kIsYCSB)
                    type = 6;
                else
                    type = 4;
            }
            else
                type = it->second.type;
            storeOptimalNode(type, key, subInitData[i].first, subInitData[i].second, subInsertData[i].first, subInsertData[i].second, i);

            totalCost += resChild.first.first + resChild.first.second;
            totalTime += resChild.first.first;
            totalSpace += resChild.first.second;

            COST.clear();
            structMap.clear();
        }
    }
    break;
    case 2:
    {
        totalTime = 44.2824;
        totalSpace += sizeof(HisType);
        for (int i = 0; i < initDataset.size(); i++)
        {
            int p = root.lrRoot.model.Predict(initDataset[i].first);
            if (subInitData[p].first == -1)
                subInitData[p].first = i;
            subInitData[p].second++;
        }
        for (int i = 0; i < findQuery.size(); i++)
        {
            int p = root.lrRoot.model.Predict(findQuery[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertQuery.size(); i++)
        {
            int p = root.lrRoot.model.Predict(insertQuery[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = GreedyAlgorithm(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
            {
                auto res0 = dp(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                auto res1 = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
            }
            int type;
            pair<bool, pair<int, int>> key = {resChild.second, {subInitData[i].first, subInitData[i].second}};
            auto it = structMap.find(key);
            if (it == structMap.end())
            {
                if (kIsYCSB)
                    type = 6;
                else
                    type = 4;
            }
            else
                type = it->second.type;
            storeOptimalNode(type, key, subInitData[i].first, subInitData[i].second, subInsertData[i].first, subInsertData[i].second, i);

            totalCost += resChild.first.first + resChild.first.second;
            totalTime += resChild.first.first;
            totalSpace += resChild.first.second;

            COST.clear();
            structMap.clear();
        }
    }
    break;
    case 3:
    {
        totalTime = 10.9438 * log(childNum) / log(2);
        totalSpace += sizeof(BSType);
        for (int i = 0; i < initDataset.size(); i++)
        {
            int p = root.lrRoot.model.Predict(initDataset[i].first);
            if (subInitData[p].first == -1)
                subInitData[p].first = i;
            subInitData[p].second++;
        }
        for (int i = 0; i < findQuery.size(); i++)
        {
            int p = root.lrRoot.model.Predict(findQuery[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertQuery.size(); i++)
        {
            int p = root.lrRoot.model.Predict(insertQuery[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = GreedyAlgorithm(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
            {
                auto res0 = dp(false, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                auto res1 = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = dp(true, subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
            }
            int type;
            pair<bool, pair<int, int>> key = {resChild.second, {subInitData[i].first, subInitData[i].second}};
            auto it = structMap.find(key);
            if (it == structMap.end())
            {
                if (kIsYCSB)
                    type = 6;
                else
                    type = 4;
            }
            else
                type = it->second.type;
            storeOptimalNode(type, key, subInitData[i].first, subInitData[i].second, subInsertData[i].first, subInsertData[i].second, i);

            totalCost += resChild.first.first + resChild.first.second;
            totalTime += resChild.first.first;
            totalSpace += resChild.first.second;

            COST.clear();
            structMap.clear();
        }
    }
    break;
    }
    UpdateLeaf();
    cout << "total cost: " << totalCost << endl;
    cout << "total time: " << totalTime << endl;
    cout << "total space: " << totalSpace << endl;

    time(&timep);
    char tmpTime1[64];
    strftime(tmpTime1, sizeof(tmpTime1), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cout << "finish time: " << tmpTime1 << endl;

    double entropy = 0.0;
    int size = initData.size();
    float p = 0;
    for (int i = 0; i < childNum; i++)
    {
        p = float(subInitData[i].second) / size;
        if (p != 0)
            entropy -= p * log(p) / log(2);
    }
    cout << "entropy: " << entropy;

    return rootType;
}

#endif