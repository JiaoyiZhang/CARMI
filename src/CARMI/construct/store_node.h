#ifndef STORE_NODE_H
#define STORE_NODE_H

#include "../../params.h"
#include "../carmi.h"
#include "../nodes/leafNode/ycsb_leaf_type.h"
#include "../nodes/leafNode/array_type.h"
#include "../nodes/leafNode/ga_type.h"
#include "../dataManager/child_array.h"
#include "../dataManager/datapoint.h"
#include "../nodes/innerNode/bs_model.h"
#include "../nodes/innerNode/lr_model.h"
#include "../nodes/innerNode/plr_model.h"
#include "../nodes/innerNode/his_model.h"
#include "dp_inner.h"
#include <float.h>
#include <vector>
#include <map>
#include <set>
using namespace std;
template <typename TYPE>
TYPE CARMI::storeInnerNode(pair<bool, pair<int, int>> key, const int left, const int size, const int insertLeft, const int insertSize, int storeIdx)
{
    auto it = structMap.find(key);
    if (it == structMap.end())
        cout << "WRONG!" << endl;
    TYPE node = TYPE();
    node.SetChildNumber(it->second.childNum);
    int optimalChildNumber = it->second.childNum;
    node.childLeft = allocateChildMemory(optimalChildNumber);
    Train(&node, left, size);

    // divide the key and query
    vector<pair<int, int>> subInitData(optimalChildNumber, {-1, 0});
    vector<pair<int, int>> subFindData(optimalChildNumber, {-1, 0});
    vector<pair<int, int>> subInsertData(optimalChildNumber, {-1, 0});
    InnerDivide<TYPE>(&node, optimalChildNumber, left, size, left, size, insertLeft, insertSize, subInitData, subFindData, subInsertData, true);

    for (int i = 0; i < optimalChildNumber; i++)
    {
        auto nowKey = it->second.child[i];
        int type;
        auto iter = structMap.find(nowKey);
        if (iter == structMap.end())
            type = 4;
        else
            type = iter->second.type;
        storeOptimalNode(type, nowKey, subInitData[i].first, subInitData[i].second, subInsertData[i].first, subInsertData[i].second, node.childLeft + i);
    }
    return node;
}

// store the optimal node into the index structure
// tmpIdx: key in the corresponding struct
void CARMI::storeOptimalNode(int optimalType, pair<bool, pair<int, int>> key, const int left, const int size, const int insertLeft, const int insertSize, int storeIdx)
{
    if (size == 0)
    {
        if (kPrimaryIndex)
        {
            auto node = YCSBLeaf();
            initYCSB(&node, left, size);
            entireChild[storeIdx].ycsbLeaf = node;
        }
        else
        {
            auto node = GappedArrayType(max(size, kThreshold));
            initGA(&node, left, size);
            entireChild[storeIdx].ga = node;

#ifdef DEBUG
            if (optimalType < 4)
                cout << "WRONG! size==0, type is:" << optimalType << endl;
#endif // DEBUG
        }
        return;
    }

    switch (optimalType)
    {
    case 0:
    {
        auto node = storeInnerNode<LRModel>(key, left, size, insertLeft, insertSize, storeIdx);
        entireChild[storeIdx].lr = node;
        break;
    }
    case 1:
    {
        auto node = storeInnerNode<PLRModel>(key, left, size, insertLeft, insertSize, storeIdx);
        entireChild[storeIdx].plr = node;
        break;
    }
    case 2:
    {
        auto node = storeInnerNode<HisModel>(key, left, size, insertLeft, insertSize, storeIdx);
        entireChild[storeIdx].his = node;
        break;
    }
    case 3:
    {
        auto node = storeInnerNode<BSModel>(key, left, size, insertLeft, insertSize, storeIdx);
        entireChild[storeIdx].bs = node;
        break;
    }
    case 4:
    {
        auto node = ArrayType(max(size, kThreshold));
        initArray(&node, left, size);
        entireChild[storeIdx].array = node;
        if (size > 0)
            scanLeaf.insert({initDataset[left].first, storeIdx});
        break;
    }
    case 5:
    {
        auto it = structMap.find(key);
        if (it == structMap.end())
            cout << "WRONG!" << endl;

        auto node = GappedArrayType(max(size, kThreshold));
        node.density = it->second.density;
        initGA(&node, left, size);
        entireChild[storeIdx].ga = node;
        if (size > 0)
            scanLeaf.insert({initDataset[left].first, storeIdx});
        break;
    }
    case 6:
    {
        auto node = YCSBLeaf();
        initYCSB(&node, left, size);
        entireChild[storeIdx].ycsbLeaf = node;
        break;
    }
    }
}

#endif // !STORE_NODE_H