#ifndef STORE_NODE_H
#define STORE_NODE_H

#include "../params.h"
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
#include "../function.h"
#include "params_struct.h"
#include <float.h>
#include <vector>
#include <map>
using namespace std;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

extern map<pair<bool, pair<int, int>>, ParamStruct> structMap;

extern vector<pair<double, double>> findActualDataset;
extern vector<pair<double, double>> insertActualDataset;

// store the optimal node into the index structure
// tmpIdx: key in the corresponding struct
int storeOptimalNode(int optimalType, pair<bool, pair<int, int>> key, const int left, const int size, const int insertLeft, const int insertSize)
{
    vector<pair<double, double>> datapoint;
    for (int i = left; i < left + size; i++)
        datapoint.push_back(findActualDataset[i]);
    vector<pair<double, double>> insertData;
    for (int i = insertLeft; i < insertLeft + insertSize; i++)
        insertData.push_back(insertActualDataset[i]);
    int idx;
    switch (optimalType)
    {
    case 0:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;
        auto node = LRType(it->second.childNum);
        LRVector.push_back(node);
        idx = LRVector.size() - 1;
        int optimalChildNumber = LRVector[idx].childNumber;
        LRVector[idx].childLeft = allocateChildMemory(optimalChildNumber);
        LRVector[idx].model.Train(datapoint, optimalChildNumber);
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        for (int i = 0; i < size; i++)
        {
            int p = LRVector[idx].model.Predict(datapoint[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i + left;
        }
        for (int i = 0; i < insertSize; i++)
        {
            int p = LRVector[idx].model.Predict(insertData[i].first);
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
            actualIdx = storeOptimalNode(type, nowKey, subLeft[i], subFindData[i], subInsertLeft[i], subInsertData[i]);
            entireChild[LRVector[idx].childLeft + i] = (type << 28) + actualIdx;
        }
        break;
    }
    case 1:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;
        auto node = NNType(it->second.childNum);
        NNVector.push_back(node);
        idx = NNVector.size() - 1;
        int optimalChildNumber = NNVector[idx].childNumber;
        NNVector[idx].childLeft = allocateChildMemory(optimalChildNumber);
        NNVector[idx].model.Train(datapoint, optimalChildNumber);
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        for (int i = 0; i < size; i++)
        {
            int p = NNVector[idx].model.Predict(datapoint[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i + left;
        }
        for (int i = 0; i < insertSize; i++)
        {
            int p = NNVector[idx].model.Predict(insertData[i].first);
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
            actualIdx = storeOptimalNode(type, nowKey, subLeft[i], subFindData[i], subInsertLeft[i], subInsertData[i]);
            entireChild[NNVector[idx].childLeft + i] = (type << 28) + actualIdx;
        }
        break;
    }
    case 2:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;
        auto node = HisType(it->second.childNum);
        HisVector.push_back(node);
        idx = HisVector.size() - 1;
        int optimalChildNumber = HisVector[idx].childNumber;
        HisVector[idx].childLeft = allocateChildMemory(optimalChildNumber);
        HisVector[idx].model.Train(datapoint, optimalChildNumber);
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        for (int i = 0; i < size; i++)
        {
            int p = HisVector[idx].model.Predict(datapoint[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i + left;
        }
        for (int i = 0; i < insertSize; i++)
        {
            int p = HisVector[idx].model.Predict(insertData[i].first);
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
            actualIdx = storeOptimalNode(type, nowKey, subLeft[i], subFindData[i], subInsertLeft[i], subInsertData[i]);
            entireChild[HisVector[idx].childLeft + i] = (type << 28) + actualIdx;
        }
        break;
    }
    case 3:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;
        auto node = BSType(it->second.childNum);
        BSVector.push_back(node);
        idx = BSVector.size() - 1;
        int optimalChildNumber = BSVector[idx].childNumber;
        BSVector[idx].childLeft = allocateChildMemory(optimalChildNumber);
        BSVector[idx].model.Train(datapoint, optimalChildNumber);
        // divide the key and query
        vector<int> subFindData(optimalChildNumber, 0);
        vector<int> subInsertData(optimalChildNumber, 0);
        vector<int> subLeft(optimalChildNumber, -1);       // {left, size}
        vector<int> subInsertLeft(optimalChildNumber, -1); // {left, size}

        for (int i = 0; i < size; i++)
        {
            int p = BSVector[idx].model.Predict(datapoint[i].first);
            subFindData[p]++;
            if (subLeft[p] == -1)
                subLeft[p] = i + left;
        }
        for (int i = 0; i < insertSize; i++)
        {
            int p = BSVector[idx].model.Predict(insertData[i].first);
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
            actualIdx = storeOptimalNode(type, nowKey, subLeft[i], subFindData[i], subInsertLeft[i], subInsertData[i]);
            entireChild[BSVector[idx].childLeft + i] = (type << 28) + actualIdx;
        }
        break;
    }
    case 4:
    {
        // choose an array node as the leaf node
        ArrayVector.push_back(ArrayType(max(size + insertSize, kMaxKeyNum)));
        idx = ArrayVector.size() - 1;
        ArrayVector[idx].SetDataset(datapoint, ArrayVector[idx].m_capacity);
        break;
    }
    case 5:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;
        GAVector.push_back(GappedArrayType(max(size + insertSize, kMaxKeyNum)));
        idx = GAVector.size() - 1;
        GAVector[idx].density = it->second.density;
        GAVector[idx].SetDataset(datapoint, GAVector[idx].capacity);
        break;
    }
    }
    return idx;
}

#endif // !STORE_NODE_H