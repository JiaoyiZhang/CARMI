#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include "greedy.h"
#include "store_node.h"
#include "dp.h"
#include "params_struct.h"
#include <map>
using namespace std;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

vector<pair<double, double>> findDatapoint;
vector<pair<double, double>> insertDatapoint;

extern pair<double, double> *entireData;

map<pair<int, int>, pair<double, double>> COST; // int:left; double:time, space
map<pair<bool, pair<int, int>>, ParamStruct> structMap;

extern int kMaxKeyNum;
extern double kRate;

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
    initEntireData(0, findData.size() + insertData.size(), false);
    initEntireChild(findData.size() + insertData.size());
    findDatapoint = findData;
    insertDatapoint = insertData;
    auto res = ChooseRoot(findData);

    vector<LRType>().swap(LRVector);
    vector<NNType>().swap(NNVector);
    vector<HisType>().swap(HisVector);
    vector<BSType>().swap(BSVector);
    vector<ArrayType>().swap(ArrayVector);
    vector<GappedArrayType>().swap(GAVector);

    COST.clear();
    structMap.clear();

    int childNum = res.second;
    int rootType = res.first;
    cout << "Construction of the root node has been completed!" << endl;
    cout << "The optimal value of root is: " << res.first << ",\tthe optimal child number is: " << res.second << endl;
    switch (rootType)
    {
    case 0:
    {
        LRVector.push_back(LRType(childNum));
        LRVector[0].childLeft = allocateChildMemory(childNum);
        LRVector[0].model.Train(findData, childNum);
        break;
    }
    case 1:
    {
        NNVector.push_back(NNType(childNum));
        NNVector[0].childLeft = allocateChildMemory(childNum);
        NNVector[0].model.Train(findData, childNum);
        break;
    }
    case 2:
    {
        HisVector.push_back(HisType(childNum));
        HisVector[0].childLeft = allocateChildMemory(childNum);
        HisVector[0].model.Train(findData, childNum);
        break;
    }
    case 3:
    {
        BSVector.push_back(BSType(childNum));
        BSVector[0].childLeft = allocateChildMemory(childNum);
        BSVector[0].model.Train(findData, childNum);
        break;
    }
    }
    double totalCost = 0.0;
    vector<pair<int, int>> subFindData(childNum, {-1, 0});   // {left, size}
    vector<pair<int, int>> subInsertData(childNum, {-1, 0}); // {left, size}
    switch (rootType)
    {
    case 0:
    {
        for (int i = 0; i < findDatapoint.size(); i++)
        {
            int p = LRVector[0].model.Predict(findDatapoint[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertDatapoint.size(); i++)
        {
            int p = LRVector[0].model.Predict(insertDatapoint[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            int idx;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second >= kMaxKeyNum)
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
            type = (structMap.find(key))->second.type;
            idx = storeOptimalNode(type, key, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);

            idx += (type << 28);
            entireChild[LRVector[0].childLeft + i] = idx;

            totalCost += resChild.first.first + resChild.first.second;

            COST.clear();
            structMap.clear();
        }
        break;
    }
    case 1:
    {
        for (int i = 0; i < findDatapoint.size(); i++)
        {
            int p = NNVector[0].model.Predict(findDatapoint[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertDatapoint.size(); i++)
        {
            int p = NNVector[0].model.Predict(insertDatapoint[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            int idx;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second >= kMaxKeyNum)
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
            type = (structMap.find(key))->second.type;
            idx = idx = storeOptimalNode(type, key, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);

            idx += (type << 28);
            entireChild[NNVector[0].childLeft + i] = idx;

            totalCost += resChild.first.first + resChild.first.second;

            COST.clear();
            structMap.clear();
        }
    }
    break;
    case 2:
    {
        for (int i = 0; i < findDatapoint.size(); i++)
        {
            int p = HisVector[0].model.Predict(findDatapoint[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertDatapoint.size(); i++)
        {
            int p = HisVector[0].model.Predict(insertDatapoint[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            int idx;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second >= kMaxKeyNum)
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
            type = (structMap.find(key))->second.type;
            idx = idx = storeOptimalNode(type, key, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);

            idx += (type << 28);
            entireChild[HisVector[0].childLeft + i] = idx;

            totalCost += resChild.first.first + resChild.first.second;

            COST.clear();
            structMap.clear();
        }
    }
    break;
    case 3:
    {
        for (int i = 0; i < findDatapoint.size(); i++)
        {
            int p = BSVector[0].model.Predict(findDatapoint[i].first);
            if (subFindData[p].first == -1)
                subFindData[p].first = i;
            subFindData[p].second++;
        }
        for (int i = 0; i < insertDatapoint.size(); i++)
        {
            int p = BSVector[0].model.Predict(insertDatapoint[i].first);
            if (subInsertData[p].first == -1)
                subInsertData[p].first = i;
            subInsertData[p].second++;
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, bool> resChild;
            int idx;
            if (subFindData[i].second + subInsertData[i].second > 4096)
                resChild = dp(false, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
            else if (subFindData[i].second + subInsertData[i].second >= kMaxKeyNum)
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
            type = (structMap.find(key))->second.type;
            idx = idx = storeOptimalNode(type, key, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);

            idx += (type << 28);
            entireChild[BSVector[0].childLeft + i] = idx;

            totalCost += resChild.first.first + resChild.first.second;

            COST.clear();
            structMap.clear();
        }
    }
    break;
    }
    cout << "total cost: " << totalCost << endl;
    return rootType;
}

#endif