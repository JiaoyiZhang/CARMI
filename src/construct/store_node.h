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
#include <set>
using namespace std;

extern map<pair<bool, pair<int, int>>, ParamStruct> structMap;

extern vector<pair<double, double>> findActualDataset;
extern vector<pair<double, double>> insertActualDataset;
extern map<double, int> scanLeaf;

set<int> storeIdxSet;

// store the optimal node into the index structure
// tmpIdx: key in the corresponding struct
void storeOptimalNode(int optimalType, pair<bool, pair<int, int>> key, const int left, const int size, const int insertLeft, const int insertSize, int storeIdx)
{
    if (storeIdxSet.find(storeIdx) == storeIdxSet.end())
        storeIdxSet.insert(storeIdx);
    else
        cout << "storeIdx is duplicated, storeIdx:" << storeIdx << endl;
    if (size == 0)
    {
        // choose an array node as the leaf node
        if (kIsYCSB)
        {
            auto node = YCSBLeaf();
            node.SetDataset(left, size);
            entireChild[storeIdx].ycsbLeaf = node;
        }
        else
        {
            auto node = ArrayType(max(size + insertSize, kThreshold));
            node.SetDataset(left, size);
            entireChild[storeIdx].array = node;
            if (optimalType < 4)
                cout << "WRONG! size==0, type is:" << optimalType << endl;
        }
        return;
    }

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
        node.Train(left, size);
        entireChild[storeIdx].lr = node;
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        int end = left + size;
        for (int i = left; i < end; i++)
        {
            int p = node.Predict(findActualDataset[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i;
        }
        int insertEnd = insertLeft + insertSize;
        for (int i = insertLeft; i < insertEnd; i++)
        {
            int p = node.Predict(insertActualDataset[i].first);
            subInsertData[p]++;
            if (subInsertLeft[p] == -1)
                subInsertLeft[p] = i;
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            auto nowKey = it->second.child[i];
            int type;
            auto iter = structMap.find(nowKey);
            if (iter == structMap.end())
                type = 4;
            else
                type = iter->second.type;
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
        node.Train(left, size);
        entireChild[storeIdx].nn = node;
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        int end = left + size;
        for (int i = left; i < end; i++)
        {
            int p = node.Predict(findActualDataset[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i;
        }
        int insertEnd = insertLeft + insertSize;
        for (int i = insertLeft; i < insertEnd; i++)
        {
            int p = node.Predict(insertActualDataset[i].first);
            subInsertData[p]++;
            if (subInsertLeft[p] == -1)
                subInsertLeft[p] = i;
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            auto nowKey = it->second.child[i];
            int type;
            auto iter = structMap.find(nowKey);
            if (iter == structMap.end())
                type = 4;
            else
                type = iter->second.type;
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
        node.Train(left, size);
        entireChild[storeIdx].his = node;
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        int end = left + size;
        for (int i = left; i < end; i++)
        {
            int p = node.Predict(findActualDataset[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i;
        }
        int insertEnd = insertLeft + insertSize;
        for (int i = insertLeft; i < insertEnd; i++)
        {
            int p = node.Predict(insertActualDataset[i].first);
            subInsertData[p]++;
            if (subInsertLeft[p] == -1)
                subInsertLeft[p] = i;
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            auto nowKey = it->second.child[i];
            int type;
            auto iter = structMap.find(nowKey);
            if (iter == structMap.end())
                type = 4;
            else
                type = iter->second.type;
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
        node.Train(left, size);
        entireChild[storeIdx].bs = node;
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        int end = left + size;
        for (int i = left; i < end; i++)
        {
            int p = node.Predict(findActualDataset[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i;
        }
        int insertEnd = insertLeft + insertSize;
        for (int i = insertLeft; i < insertEnd; i++)
        {
            int p = node.Predict(insertActualDataset[i].first);
            subInsertData[p]++;
            if (subInsertLeft[p] == -1)
                subInsertLeft[p] = i;
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            auto nowKey = it->second.child[i];
            int type;
            auto iter = structMap.find(nowKey);
            if (iter == structMap.end())
                type = 4;
            else
                type = iter->second.type;
            storeOptimalNode(type, nowKey, subLeft[i], subFindData[i], subInsertLeft[i], subInsertData[i], node.childLeft + i);
        }
        break;
    }
    case 4:
    {
        // choose an array node as the leaf node
        auto node = ArrayType(max(size + insertSize, kThreshold));
        node.SetDataset(left, size);
        entireChild[storeIdx].array = node;
        if (size > 0)
            scanLeaf.insert({findActualDataset[left].first, storeIdx});
        break;
    }
    case 5:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;

        auto node = GappedArrayType(max(size + insertSize, kThreshold));
        node.density = it->second.density;
        node.SetDataset(left, size);
        entireChild[storeIdx].ga = node;
        if (size > 0)
            scanLeaf.insert({findActualDataset[left].first, storeIdx});
        break;
    }
    case 6:
    {
        auto node = YCSBLeaf();
        node.SetDataset(left, size);
        entireChild[storeIdx].ycsbLeaf = node;
        break;
    }
    }
}

#endif // !STORE_NODE_H