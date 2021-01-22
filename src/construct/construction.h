#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include "choose_root.h"
#include "store_node.h"
#include "dp.h"
#include "params_struct.h"
#include "update_leaf.h"
#include <vector>
#include <map>
using namespace std;

extern vector<BaseNode> entireChild;

vector<pair<double, double>> findDatapoint;
vector<pair<double, double>> insertDatapoint;

extern pair<double, double> *entireData;

map<pair<int, int>, pair<double, double>> COST; // int:left; double:time, space
map<pair<bool, pair<int, int>>, ParamStruct> structMap;

extern int kMaxKeyNum;
extern set<int> storeIdxSet;
extern double kRate;
map<double, int> scanLeaf;
// main function of construction
// return the type of root
// findDatapoint: the dataset used to initialize the index
// insertDatapoint: the dataset to be inserted into the index
// readCnt: the number of READ corresponding to each key
// writeCnt: the number of WRITE corresponding to each key
int Construction(const vector<pair<double, double>> &findData, const vector<pair<double, double>> &insertData)
{
    cout << endl;
    cout << "-------------------------------" << endl;
    cout << "Start construction!" << endl;
    storeIdxSet.clear();

    time_t timep;
    time(&timep);
    char tmpTime[64];
    strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cout << "\nTEST time: " << tmpTime << endl;

    initEntireData(0, findData.size() + insertData.size(), false);
    initEntireChild(findData.size() + insertData.size());
    findDatapoint = findData;
    insertDatapoint = insertData;
    auto res = ChooseRoot(findData);

    COST.clear();
    structMap.clear();
    scanLeaf.clear();

    int childNum = res.second;
    int rootType = res.first;
    kInnerNodeID = rootType;
    cout << "Construction of the root node has been completed!" << endl;
    cout << "The optimal value of root is: " << res.first << ",\tthe optimal child number is: " << res.second << endl;
    switch (rootType)
    {
    case 0:
    {
        lrRoot = LRType(childNum);
        lrRoot.childLeft = allocateChildMemory(childNum);
        lrRoot.model.Train(findData, childNum);
        break;
    }
    case 1:
    {
        nnRoot = NNType(childNum);
        nnRoot.childLeft = allocateChildMemory(childNum);
        nnRoot.model.Train(findData, childNum);
        break;
    }
    case 2:
    {
        hisRoot = HisType(childNum);
        hisRoot.childLeft = allocateChildMemory(childNum);
        hisRoot.model.Train(findData, childNum);
        break;
    }
    case 3:
    {
        bsRoot = BSType(childNum);
        bsRoot.childLeft = allocateChildMemory(childNum);
        bsRoot.model.Train(findData, childNum);
        break;
    }
    }
    double totalCost = 0.0;
    double totalTime = 0.0;
    double totalSpace = 0.0;
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
        for (int i = 0; i < findDatapoint.size(); i++)
        {
            int p = lrRoot.model.Predict(findDatapoint[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertDatapoint.size(); i++)
        {
            int p = lrRoot.model.Predict(insertDatapoint[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            // if (i % 10000 == 0)
            // {
            //     cout << "construct child " << i << ":\tsize:" << subFindData[i].second + subInsertData[i].second << endl;

            //     time_t timep;
            //     time(&timep);
            //     char tmpTime[64];
            //     strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
            //     cout << "start time: " << tmpTime << endl;
            // }
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
            {
                auto res0 = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                auto res1 = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
            }
            int type;
            pair<bool, pair<int, int>> key = {resChild.second, {subFindData[i].first, subFindData[i].second}};
            auto it = structMap.find(key);
            if (it == structMap.end())
                type = 4;
            else
                type = it->second.type;
            // if (i % 10000 == 0)
            // {
            //     cout << "construct child " << i << " over!\ttype is:" << type << endl;

            //     time_t timep;
            //     time(&timep);
            //     char tmpTime[64];
            //     strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
            //     cout << "over time: " << tmpTime << endl;
            // }
            storeOptimalNode(type, key, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second, i);
            // if (i % 10000 == 0)
            // {
            //     cout << "store child " << i << " over!" << endl;

            //     time_t timep;
            //     time(&timep);
            //     char tmpTime[64];
            //     strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
            //     cout << "over time: " << tmpTime << endl;
            // }

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
        totalSpace += sizeof(NNType);
        for (int i = 0; i < findDatapoint.size(); i++)
        {
            int p = nnRoot.model.Predict(findDatapoint[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertDatapoint.size(); i++)
        {
            int p = nnRoot.model.Predict(insertDatapoint[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
            {
                auto res0 = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                auto res1 = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
            }
            int type;
            pair<bool, pair<int, int>> key = {resChild.second, {subFindData[i].first, subFindData[i].second}};
            auto it = structMap.find(key);
            if (it == structMap.end())
                type = 4;
            else
                type = it->second.type;
            storeOptimalNode(type, key, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second, i);

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
        for (int i = 0; i < findDatapoint.size(); i++)
        {
            int p = hisRoot.model.Predict(findDatapoint[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertDatapoint.size(); i++)
        {
            int p = hisRoot.model.Predict(insertDatapoint[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
            {
                auto res0 = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                auto res1 = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
            }
            int type;
            pair<bool, pair<int, int>> key = {resChild.second, {subFindData[i].first, subFindData[i].second}};
            auto it = structMap.find(key);
            if (it == structMap.end())
                type = 4;
            else
                type = it->second.type;
            storeOptimalNode(type, key, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second, i);

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
        for (int i = 0; i < findDatapoint.size(); i++)
        {
            int p = bsRoot.model.Predict(findDatapoint[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertDatapoint.size(); i++)
        {
            int p = bsRoot.model.Predict(insertDatapoint[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = GreedyAlgorithm(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second > kMaxKeyNum)
            {
                auto res0 = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
                auto res1 = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);  // construct a leaf node
                if (res0.first.first + res0.first.second > res1.first.first + res1.first.second)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = dp(true, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
            }
            int type;
            pair<bool, pair<int, int>> key = {resChild.second, {subFindData[i].first, subFindData[i].second}};
            auto it = structMap.find(key);
            if (it == structMap.end())
                type = 4;
            else
                type = it->second.type;
            storeOptimalNode(type, key, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second, i);

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

    return rootType;
}

#endif