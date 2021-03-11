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
TYPE CARMI::storeInnerNode(int storeIdx, const MapKey &key, const IndexPair &range)
{
    auto it = structMap.find(key);
    if (it == structMap.end())
        cout << "WRONG!" << endl;

    int optimalChildNumber = it->second.childNum;
    SubDataset subDataset = SubDataset(optimalChildNumber);
    TYPE node = InnerDivideAll<TYPE>(optimalChildNumber, range, &subDataset);
    node.childLeft = allocateChildMemory(optimalChildNumber);

    for (int i = 0; i < optimalChildNumber; i++)
    {
        auto nowKey = it->second.child[i];
        int type;
        auto iter = structMap.find(nowKey);
        if (iter == structMap.end())
            type = 4;
        else
            type = iter->second.type;
        IndexPair subRange(subDataset.subInit[i], subDataset.subFind[i], subDataset.subInsert[i]);
        storeOptimalNode(node.childLeft + i, type, nowKey, subRange);
    }
    return node;
}

void CARMI::storeOptimalNode(int storeIdx, int optimalType, const MapKey key, const IndexPair &range)
{
    if (range.initRange.size == 0)
    {
        if (kPrimaryIndex)
        {
            auto node = YCSBLeaf();
            initYCSB(&node, range.initRange.left, range.initRange.size);
            entireChild[storeIdx].ycsbLeaf = node;
        }
        else
        {
            auto node = GappedArrayType(kThreshold);
            initGA(&node, range.initRange.left, range.initRange.size);
            entireChild[storeIdx].ga = node;
        }
#ifdef DEBUG
        if (optimalType < 4)
            cout << "WRONG! size==0, type is:" << optimalType << endl;
#endif // DEBUG
        return;
    }

    switch (optimalType)
    {
    case 0:
    {
        auto node = storeInnerNode<LRModel>(storeIdx, key, range);
        entireChild[storeIdx].lr = node;
        break;
    }
    case 1:
    {
        auto node = storeInnerNode<PLRModel>(storeIdx, key, range);
        entireChild[storeIdx].plr = node;
        break;
    }
    case 2:
    {
        auto node = storeInnerNode<HisModel>(storeIdx, key, range);
        entireChild[storeIdx].his = node;
        break;
    }
    case 3:
    {
        auto node = storeInnerNode<BSModel>(storeIdx, key, range);
        entireChild[storeIdx].bs = node;
        break;
    }
    case 4:
    {
        auto node = ArrayType(max(range.initRange.size, kThreshold));
        initArray(&node, range.initRange.left, range.initRange.size);
        entireChild[storeIdx].array = node;
        if (range.initRange.size > 0)
            scanLeaf.insert({initDataset[range.initRange.left].first, storeIdx});
        break;
    }
    case 5:
    {
        auto it = structMap.find(key);
        auto node = GappedArrayType(kThreshold);
        node.density = it->second.density;
        initGA(&node, range.initRange.left, range.initRange.size);
        entireChild[storeIdx].ga = node;
        if (range.initRange.size > 0)
            scanLeaf.insert({initDataset[range.initRange.left].first, storeIdx});
        break;
    }
    case 6:
    {
        auto node = YCSBLeaf();
        initYCSB(&node, range.initRange.left, range.initRange.size);
        entireChild[storeIdx].ycsbLeaf = node;
        break;
    }
    }
}

#endif // !STORE_NODE_H