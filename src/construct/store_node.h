#ifndef STORE_NODE_H
#define STORE_NODE_H

#include "../params.h"
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
#include "../func/function.h"
#include "params_struct.h"
#include <float.h>
#include <vector>
#include <map>
using namespace std;

extern map<pair<bool, pair<int, int>>, ParamStruct> structMap;

extern vector<pair<double, double>> findActualDataset;
extern vector<pair<double, double>> insertActualDataset;

// store the optimal node into the index structure
// tmpIdx: key in the corresponding struct
void storeOptimalNode(int optimalType, pair<bool, pair<int, int>> key, const int left, const int size, const int insertLeft, const int insertSize, int storeIdx)
{
    vector<pair<double, double>> datapoint;
    for (int i = left; i < left + size; i++)
        datapoint.push_back(findActualDataset[i]);
    vector<pair<double, double>> insertData;
    for (int i = insertLeft; i < insertLeft + insertSize; i++)
        insertData.push_back(insertActualDataset[i]);
    // int idx;
    switch (optimalType)
    {
    case 0:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;
        auto node = LRModel();
        node.SetChildNumber(it->second.childNum);
        int optimalChildNumber = it->second.childNum;
        node.childLeft = allocateChildMemory(optimalChildNumber);
        node.Train(datapoint);
        entireChild[storeIdx].lr = node;
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        for (int i = 0; i < size; i++)
        {
            int p = node.Predict(datapoint[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i + left;
        }
        for (int i = 0; i < insertSize; i++)
        {
            int p = node.Predict(insertData[i].first);
            subInsertData[p]++;
            if (subInsertLeft[p] == -1)
                subInsertLeft[p] = i + insertLeft;
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            auto nowChild = it->second.child[i];
            pair<bool, pair<int, int>> nowKey = nowChild;
            int actualIdx, type;
            type = (structMap.find(nowKey))->second.type;
            storeOptimalNode(type, nowKey, subLeft[i], subFindData[i], subInsertLeft[i], subInsertData[i], node.childLeft + i);
        }
        break;
    }
    case 1:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;
        auto node = NNModel();
        node.SetChildNumber(it->second.childNum);
        int optimalChildNumber = it->second.childNum;
        node.childLeft = allocateChildMemory(optimalChildNumber);
        node.Train(datapoint);
        entireChild[storeIdx].nn = node;
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        for (int i = 0; i < size; i++)
        {
            int p = node.Predict(datapoint[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i + left;
        }
        for (int i = 0; i < insertSize; i++)
        {
            int p = node.Predict(insertData[i].first);
            subInsertData[p]++;
            if (subInsertLeft[p] == -1)
                subInsertLeft[p] = i + insertLeft;
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            auto nowChild = it->second.child[i];
            pair<bool, pair<int, int>> nowKey = nowChild;
            int actualIdx, type;
            type = (structMap.find(nowKey))->second.type;
            storeOptimalNode(type, nowKey, subLeft[i], subFindData[i], subInsertLeft[i], subInsertData[i], node.childLeft + i);
        }
        break;
    }
    case 2:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;
        auto node = HisModel();
        node.SetChildNumber(it->second.childNum);
        int optimalChildNumber = it->second.childNum;
        node.childLeft = allocateChildMemory(optimalChildNumber);
        node.Train(datapoint);
        entireChild[storeIdx].his = node;
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        for (int i = 0; i < size; i++)
        {
            int p = node.Predict(datapoint[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i + left;
        }
        for (int i = 0; i < insertSize; i++)
        {
            int p = node.Predict(insertData[i].first);
            subInsertData[p]++;
            if (subInsertLeft[p] == -1)
                subInsertLeft[p] = i + insertLeft;
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            auto nowChild = it->second.child[i];
            pair<bool, pair<int, int>> nowKey = nowChild;
            int actualIdx, type;
            type = (structMap.find(nowKey))->second.type;
            storeOptimalNode(type, nowKey, subLeft[i], subFindData[i], subInsertLeft[i], subInsertData[i], node.childLeft + i);
        }
        break;
    }
    case 3:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;
        auto node = BSModel();
        node.SetChildNumber(it->second.childNum);
        int optimalChildNumber = it->second.childNum;
        node.childLeft = allocateChildMemory(optimalChildNumber);
        node.Train(datapoint);
        entireChild[storeIdx].bs = node;
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        for (int i = 0; i < size; i++)
        {
            int p = node.Predict(datapoint[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i + left;
        }
        for (int i = 0; i < insertSize; i++)
        {
            int p = node.Predict(insertData[i].first);
            subInsertData[p]++;
            if (subInsertLeft[p] == -1)
                subInsertLeft[p] = i + insertLeft;
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            auto nowChild = it->second.child[i];
            pair<bool, pair<int, int>> nowKey = nowChild;
            int actualIdx, type;
            type = (structMap.find(nowKey))->second.type;
            storeOptimalNode(type, nowKey, subLeft[i], subFindData[i], subInsertLeft[i], subInsertData[i], node.childLeft + i);
        }
        break;
    }
    case 4:
    {
        // choose an array node as the leaf node
        auto node = ArrayType(max(size + insertSize, kMaxKeyNum));
        node.SetDataset(datapoint, node.m_capacity);
        entireChild[storeIdx].array = node;
        break;
    }
    case 5:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;

        auto node = GappedArrayType(max(size + insertSize, kMaxKeyNum));
        node.density = it->second.density;
        node.SetDataset(datapoint, node.capacity);
        entireChild[storeIdx].ga = node;
        break;
    }
    }
}

#endif // !STORE_NODE_H