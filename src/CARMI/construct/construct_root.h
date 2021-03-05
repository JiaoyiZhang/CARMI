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
    SingleDataRange range;
    range.left = 0;
    range.size = initDataset.size();
    NodePartition<ModelType>(&root.model, &perSize, &range, initDataset);
    double entropy = CalculateEntropy(perSize.subSize, initDataset.size(), c);
    double cost = (time + float(kRate * space)) / entropy;

    if (cost <= optimalCost)
    {
        optimalCost = cost;
        rootStruct = new RootStruct(type, c);
    }
}

RootStruct *CARMI::ChooseRoot()
{
    double OptimalValue = DBL_MAX;
    RootStruct *rootStruct;
    for (int c = 1024; c <= initDataset.size() * 10; c *= 2)
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

template <typename TYPE>
TYPE *CARMI::ConstructRoot(RootStruct *rootStruct, SubDataset *subDataset, DataRange *range, int &childLeft)
{
    TYPE *root = new TYPE(rootStruct->rootChildNum);
    childLeft = allocateChildMemory(rootStruct->rootChildNum);
    root->model.Train(initDataset, rootStruct->rootChildNum);

    NodePartition<TYPE>(root, &(subDataset->subInit), &(range->initRange), initDataset);
    subDataset->subFind = subDataset->subInit;
    NodePartition<TYPE>(root, &(subDataset->subInsert), &(range->insertRange), insertQuery);
    return root;
}

SubDataset *CARMI::StoreRoot(RootStruct *rootStruct, NodeCost *nodeCost)
{
    DataRange dataRange;
    SubDataset *subDataset = new SubDataset(rootStruct->rootChildNum);
    int childLeft;
    switch (rootStruct->rootType)
    {
    case 0:
    {
        nodeCost->time = LRRootTime;
        nodeCost->space += sizeof(LRType);
        root.lrRoot = *ConstructRoot<LRType>(rootStruct, subDataset, &dataRange, childLeft);
        break;
    }
    case 1:
    {
        nodeCost->time = PLRRootTime;
        nodeCost->space += sizeof(PLRType);
        root.plrRoot = *ConstructRoot<PLRType>(rootStruct, subDataset, &dataRange, childLeft);
        break;
    }
    case 2:
    {
        nodeCost->time = HisRootTime;
        nodeCost->space += sizeof(HisType);
        root.hisRoot = *ConstructRoot<HisType>(rootStruct, subDataset, &dataRange, childLeft);
        break;
    }
    case 3:
    {
        nodeCost->time = 10.9438 * log(rootStruct->rootChildNum) / log(2);
        nodeCost->space += sizeof(BSType);
        root.bsRoot = *ConstructRoot<BSType>(rootStruct, subDataset, &dataRange, childLeft);
        break;
    }
    }
    return subDataset;
}
#endif // !CONSTRUCT_ROOT_H