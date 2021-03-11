/**
 * @file construction.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include <map>
#include <vector>

#include "../carmi.h"
#include "construct_root.h"
#include "structures.h"
using namespace std;

/**
 * @brief construct each subtree using dp/greedy
 * @param rootStruct the type and childNumber of root
 * @param subDataset the left and size of data points in each child node
 * @param nodeCost the space, time, cost of the index
 */
inline void CARMI::ConstructSubTree(const RootStruct &rootStruct,
                                    const SubDataset &subDataset,
                                    NodeCost *nodeCost) {
  for (int i = 0; i < rootStruct.rootChildNum; i++) {
    COST.insert({emptyRange, emptyCost});
    structMap.insert({(MapKey){false, emptyRange}, leafP});

    NodeCost resChild;
    IndexPair range(subDataset.subInit[i], subDataset.subFind[i],
                    subDataset.subInsert[i]);
    if (subDataset.subInit[i].size + subDataset.subInsert[i].size >
        kAlgorithmThreshold)
      resChild = GreedyAlgorithm(range);
    else
      resChild = dp(range);
    int type;
    MapKey key = {resChild.isInnerNode,
                  {subDataset.subInit[i].size, subDataset.subInsert[i].size}};
    auto it = structMap.find(key);
    type = it->second.type;

    storeOptimalNode(i, type, key, range);

    nodeCost->cost += resChild.space + resChild.time;
    nodeCost->time += resChild.time;
    nodeCost->space += resChild.space;

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
inline int CARMI::Construction(const vector<pair<double, double>> &initData,
                               const vector<pair<double, double>> &findData,
                               const vector<pair<double, double>> &insertData) {
  NodeCost nodeCost = {0, 0, 0, true};
  RootStruct res = ChooseRoot();
  // RootStruct res = RootStruct(0, 131072);
  rootType = res.rootType;
  SubDataset subDataset = StoreRoot(res, &nodeCost);

  ConstructSubTree(res, subDataset, &nodeCost);
  UpdateLeaf();

  if (kPrimaryIndex) {
    vector<pair<double, double>> tmp(100000, {DBL_MIN, DBL_MIN});
    entireData.insert(entireData.end(), tmp.begin(), tmp.end());
    nowDataSize += 100000;
  }
  vector<pair<double, double>>().swap(initDataset);
  vector<pair<double, double>>().swap(findQuery);
  vector<pair<double, double>>().swap(insertQuery);
  entireData.erase(entireData.begin() + nowDataSize + reservedSpace,
                   entireData.end());

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