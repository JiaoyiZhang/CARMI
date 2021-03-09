#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include "structures.h"
#include "construct_root.h"
#include "../carmi.h"
#include <vector>
#include <map>
using namespace std;

inline void CARMI::ConstructSubTree(RootStruct *rootStruct, SubDataset *subDataset, NodeCost *nodeCost)
{
    for (int i = 0; i < rootStruct->rootChildNum; i++)
    {
        NodeCost emptyCost = {0, 0, 0, false};
        SingleDataRange emptyRange = {-1, 0};
        COST.insert({emptyRange, emptyCost});
        ParamStruct leafP;
        leafP.type = 5;
        leafP.density = 0.5;
        structMap.insert({{false, {-1, 0}}, leafP});

        NodeCost resChild;
        DataRange *range = new DataRange(subDataset->subInit->subLeft[i], subDataset->subInit->subSize[i], subDataset->subFind->subLeft[i], subDataset->subFind->subSize[i], subDataset->subInsert->subLeft[i], subDataset->subInsert->subSize[i]);
        if (subDataset->subInit->subSize[i] + subDataset->subInsert->subSize[i] > kAlgorithmThreshold)
            resChild = GreedyAlgorithm(range);
        else
            resChild = dp(range);
        int type;
        MapKey key = {resChild.isInnerNode, {subDataset->subInit->subLeft[i], subDataset->subInit->subSize[i]}};
        auto it = structMap.find(key);
        type = it->second.type;

        storeOptimalNode(type, &key, range, i);

        nodeCost->cost += resChild.space + resChild.time;
        nodeCost->time += resChild.time;
        nodeCost->space += resChild.space;

        delete range;
        COST.clear();
        structMap.clear();
    }
}

/**
 * @brief main function of construction
 * @param initData the dataset used to initialize the index
 * @param findData the find queries used to training CARMI
 * @param insertData the insert queries used to training CARMI
 * @return the type of root
 */
inline int CARMI::Construction(const vector<pair<double, double>> &initData, const vector<pair<double, double>> &findData, const vector<pair<double, double>> &insertData)
{
    NodeCost nodeCost = {0, 0, 0, true};
    // RootStruct *res = ChooseRoot();
    RootStruct *res = new RootStruct(0, 131072);
    rootType = res->rootType;
    SubDataset *subDataset = StoreRoot(res, &nodeCost);

    ConstructSubTree(res, subDataset, &nodeCost);
    UpdateLeaf();

    if (kPrimaryIndex)
    {
        vector<pair<double, double>> tmp(100000, {DBL_MIN, DBL_MIN});
        entireData.insert(entireData.end(), tmp.begin(), tmp.end());
        nowDataSize += 100000;
    }
    vector<pair<double, double>>().swap(initDataset);
    vector<pair<double, double>>().swap(findQuery);
    vector<pair<double, double>>().swap(insertQuery);
    entireData.erase(entireData.begin() + nowDataSize + reservedSpace, entireData.end());

    COST.clear();
    structMap.clear();
    scanLeaf.clear();

#ifdef DEBUG
    cout << "Construction over!" << endl;
    cout << "total cost: " << nodeCost.cost << endl;
    cout << "total time: " << nodeCost.time << endl;
    cout << "total space: " << nodeCost.space << endl;
#endif
    return rootType;
}

#endif