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
#include <float.h>
#include <vector>
using namespace std;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

extern vector<LRType> tmpLRVec;
extern vector<NNType> tmpNNVec;
extern vector<HisType> tmpHisVec;
extern vector<BSType> tmpBSVec;
extern vector<ArrayType> tmpArrayVec;
extern vector<GappedArrayType> tmpGAVec;

// store the optimal node into the index structure
// tmpIdx: idx in tmpVector
int storeOptimalNode(int optimalType, int tmpIdx, const vector<pair<double, double>> &findData)
{
    int idx;
    switch (optimalType)
    {
    case 0:
    {
        LRVector.push_back(tmpLRVec[tmpIdx]);
        idx = LRVector.size() - 1;
        int optimalChildNumber = LRVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
            subFindData.push_back(tmp);
        for (int i = 0; i < findData.size(); i++)
        {
            int p = LRVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = LRVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i]);
            LRVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        break;
    }
    case 1:
    {
        NNVector.push_back(tmpNNVec[tmpIdx]);
        idx = NNVector.size() - 1;
        int optimalChildNumber = NNVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
            subFindData.push_back(tmp);
        for (int i = 0; i < findData.size(); i++)
        {
            int p = NNVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = NNVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i]);
            NNVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        break;
    }
    case 2:
    {
        HisVector.push_back(tmpHisVec[tmpIdx]);
        idx = HisVector.size() - 1;
        int optimalChildNumber = HisVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
            subFindData.push_back(tmp);
        for (int i = 0; i < findData.size(); i++)
        {
            int p = HisVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = HisVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i]);
            HisVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        break;
    }
    case 3:
    {
        BSVector.push_back(tmpBSVec[tmpIdx]);
        idx = BSVector.size() - 1;
        int optimalChildNumber = BSVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
            subFindData.push_back(tmp);
        for (int i = 0; i < findData.size(); i++)
        {
            int p = BSVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }
        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = BSVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i]);
            BSVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        break;
    }
    case 4:
    {
        // choose an array node as the leaf node
        ArrayVector.push_back(tmpArrayVec[tmpIdx]);
        idx = ArrayVector.size() - 1;
        ArrayVector[idx].SetDataset(findData, ArrayVector[idx].m_capacity);
        break;
    }
    case 5:
    {
        GAVector.push_back(tmpGAVec[tmpIdx]);
        idx = GAVector.size() - 1;
        GAVector[idx].SetDataset(findData, GAVector[idx].capacity);
        break;
    }
    }
    return idx;
}

#endif // !STORE_NODE_H