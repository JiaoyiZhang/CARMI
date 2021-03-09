#ifndef CONSTRUCT_ROOT_H
#define CONSTRUCT_ROOT_H

#include "../carmi.h"
#include "store_node.h"
#include "structures.h"
#include "dp.h"
using namespace std;

template <typename TYPE, typename ModelType>
void CARMI::CheckRoot(int c, int type, double &optimalCost, double time, RootStruct *rootStruct)
{
    SubSingleDataset perSize(c);
    double space = 64.0 * c / 1024 / 1024;

    TYPE root = TYPE(c);
    root.model.Train(initDataset, c);
    SingleDataRange range(0, initDataset.size());
    NodePartition<ModelType>(&root.model, &perSize, &range, initDataset);
    double entropy = CalculateEntropy(perSize.subSize, initDataset.size(), c);
    double cost = (time + float(kRate * space)) / entropy;

    if (cost <= optimalCost)
    {
        optimalCost = cost;
        rootStruct->rootChildNum = c;
        rootStruct->rootType = type;
    }
}

RootStruct *CARMI::ChooseRoot()
{
    double OptimalValue = DBL_MAX;
    RootStruct *rootStruct = new RootStruct(0, 0);
    for (int c = 2; c <= initDataset.size() * 10; c *= 2)
    {
        CheckRoot<LRType, LinearRegression>(c, 0, OptimalValue, LRRootTime, rootStruct);
        CheckRoot<PLRType, PiecewiseLR>(c, 1, OptimalValue, PLRRootTime, rootStruct);
        CheckRoot<HisType, HistogramModel>(c, 2, OptimalValue, HisRootTime, rootStruct);
        CheckRoot<BSType, BinarySearchModel>(c, 3, OptimalValue, 10.9438 * log(c) / log(2), rootStruct);
    }
#ifdef DEBUG
    cout << "Best type is: " << rootStruct->rootType << "\tbest childNumber: " << rootStruct->rootChildNum << "\tOptimal Value: " << OptimalValue << endl;
#endif
    return rootStruct;
}

template <typename TYPE, typename ModelType>
TYPE *CARMI::ConstructRoot(RootStruct *rootStruct, SubDataset *subDataset, DataRange *range, int &childLeft)
{
    TYPE *root = new TYPE(rootStruct->rootChildNum);
    childLeft = allocateChildMemory(rootStruct->rootChildNum);
    root->model.Train(initDataset, rootStruct->rootChildNum);

    NodePartition<ModelType>(&root->model, (subDataset->subInit), &(range->initRange), initDataset);
    subDataset->subFind->subLeft = subDataset->subInit->subLeft;
    subDataset->subFind->subSize = subDataset->subInit->subSize;
    NodePartition<ModelType>(&root->model, (subDataset->subInsert), &(range->insertRange), insertQuery);
    return root;
}

SubDataset *CARMI::StoreRoot(RootStruct *rootStruct, NodeCost *nodeCost)
{
    DataRange dataRange(0, initDataset.size(), 0, findQuery.size(), 0, insertQuery.size());
    SubDataset *subDataset = new SubDataset(rootStruct->rootChildNum);
    int childLeft;
    switch (rootStruct->rootType)
    {
    case 0:
    {
        nodeCost->time = LRRootTime;
        nodeCost->space += sizeof(LRType);
        root.lrRoot = *ConstructRoot<LRType, LinearRegression>(rootStruct, subDataset, &dataRange, childLeft);
        break;
    }
    case 1:
    {
        nodeCost->time = PLRRootTime;
        nodeCost->space += sizeof(PLRType);
        root.plrRoot = *ConstructRoot<PLRType, PiecewiseLR>(rootStruct, subDataset, &dataRange, childLeft);
        break;
    }
    case 2:
    {
        nodeCost->time = HisRootTime;
        nodeCost->space += sizeof(HisType);
        root.hisRoot = *ConstructRoot<HisType, HistogramModel>(rootStruct, subDataset, &dataRange, childLeft);
        break;
    }
    case 3:
    {
        nodeCost->time = 10.9438 * log(rootStruct->rootChildNum) / log(2);
        nodeCost->space += sizeof(BSType);
        root.bsRoot = *ConstructRoot<BSType, BinarySearchModel>(rootStruct, subDataset, &dataRange, childLeft);
        break;
    }
    }
    return subDataset;
}
#endif // !CONSTRUCT_ROOT_H