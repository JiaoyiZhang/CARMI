#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include "root_struct.h"
#include "params_struct.h"
#include "choose_root.h"
#include "construct_root.h"
#include "update_leaf.h"
#include "../carmi.h"
#include <vector>
#include <map>
using namespace std;

// main function of construction
// return the type of root
// findDatapoint: the dataset used to initialize the index
// insertDatapoint: the dataset to be inserted into the index
// readCnt: the number of READ corresponding to each key
// writeCnt: the number of WRITE corresponding to each key
inline int CARMI::Construction(const vector<pair<double, double>> &initData, const vector<pair<double, double>> &findData, const vector<pair<double, double>> &insertData)
{
#ifdef DEBUG
    cout << endl;
    cout << "-------------------------------" << endl;
    cout << "Start construction!" << endl;
    querySize = findData.size() + insertData.size();
    time_t timep;
    time(&timep);
    char tmpTime[64];
    strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cout << "\nTEST time: " << tmpTime << endl;
#endif

    if (initDataset.size() != initData.size())
    {
        initDataset = initData;
        findQuery = findData;
        insertQuery = insertData;
    }

    if (!kPrimaryIndex)
        initEntireData(0, initDataset.size(), false);
    initEntireChild(initDataset.size());

    auto res = ChooseRoot();

    COST.clear();
    structMap.clear();
    scanLeaf.clear();

    int childNum = res.rootChildNum;
    rootType = res.rootType;
    kInnerNodeID = rootType;

#ifdef DEBUG
    cout << "Construction of the root node has been completed!" << endl;
    cout << "The optimal type of root is: " << res.rootType << ",\tthe optimal child number is: " << res.rootChildNum << endl;
#endif

    double totalCost = 0.0;
    double totalTime = 0.0;
    double totalSpace = 0.0;

#ifdef DEBUG
    time(&timep);
    char tmpTime2[64];
    strftime(tmpTime2, sizeof(tmpTime2), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cout << "Root time: " << tmpTime2 << endl;
#endif

    COST.insert({{-1, 0}, {0, 0}});
    ParamStruct leafP;
    leafP.type = 5;
    leafP.density = 0.5;
    structMap.insert({{false, {-1, 0}}, leafP});

    switch (rootType)
    {
    case 0:
    {
        root.lrRoot = LRType(childNum);
        root.lrRoot.childLeft = allocateChildMemory(childNum);
        root.lrRoot.model.Train(initDataset, childNum);
        totalTime = 12.7013;
        totalSpace += sizeof(LRType);
        ConstructRoot<LRType>(&root.lrRoot, childNum, totalCost, totalTime, totalSpace);
        break;
    }
    case 1:
    {
        root.plrRoot = PLRType(childNum);
        root.plrRoot.childLeft = allocateChildMemory(childNum);
        root.plrRoot.model.Train(initDataset, childNum);
        totalTime = 39.6429;
        totalSpace += sizeof(PLRType);
        ConstructRoot<PLRType>(&root.plrRoot, childNum, totalCost, totalTime, totalSpace);
        break;
    }
    case 2:
    {
        root.hisRoot = HisType(childNum);
        root.hisRoot.childLeft = allocateChildMemory(childNum);
        root.hisRoot.model.Train(initDataset, childNum);
        totalTime = 44.2824;
        totalSpace += sizeof(HisType);
        ConstructRoot<HisType>(&root.hisRoot, childNum, totalCost, totalTime, totalSpace);
        break;
    }
    case 3:
    {
        root.bsRoot = BSType(childNum);
        root.bsRoot.childLeft = allocateChildMemory(childNum);
        root.bsRoot.model.Train(initDataset, childNum);
        totalTime = 10.9438 * log(childNum) / log(2);
        totalSpace += sizeof(BSType);
        ConstructRoot<BSType>(&root.bsRoot, childNum, totalCost, totalTime, totalSpace);
        break;
    }
    }
    UpdateLeaf();

#ifdef DEBUG
    cout << "total cost: " << totalCost << endl;
    cout << "total time: " << totalTime << endl;
    cout << "total space: " << totalSpace << endl;

    time(&timep);
    char tmpTime1[64];
    strftime(tmpTime1, sizeof(tmpTime1), "%Y-%m-%d %H:%M:%S", localtime(&timep));
    cout << "finish time: " << tmpTime1 << endl;
#endif
    vector<pair<double, double>> tmp(100000, {DBL_MIN, DBL_MIN});
    initDataset.insert(initDataset.end(), tmp.begin(), tmp.end());
    if (!kPrimaryIndex)
        vector<pair<double, double>>().swap(initDataset);
    vector<pair<double, double>>().swap(findQuery);
    vector<pair<double, double>>().swap(insertQuery);
    entireData.erase(entireData.begin() + nowDataSize + 1024 * 1024, entireData.end());
    return rootType;
}

#endif