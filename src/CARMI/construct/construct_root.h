/**
 * @file construct_root.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CONSTRUCT_ROOT_H
#define CONSTRUCT_ROOT_H

#include "../carmi.h"
#include "dp.h"
#include "store_node.h"
#include "structures.h"
using namespace std;

/**
 * @brief determine whether the current setting is
 *        better than the previous optimal setting (more details)
 * @param c the child number of this node
 * @param type the type of this node
 * @param optimalCost the previous optimal setting
 * @param time the time cost of this node
 * @param rootStruct used to record the optimal setting
 */
template <typename TYPE, typename ModelType>
void CARMI::CheckRoot(int c, int type, double time, double *optimalCost,
                      RootStruct *rootStruct) {
  vector<DataRange> perSize(c, emptyRange);
  double space = BaseNodeSpace * c;

  TYPE root = TYPE(c);
  root.model.Train(initDataset, c);
  DataRange range(0, initDataset.size());
  NodePartition<ModelType>(root.model, range, initDataset, &perSize);
  double entropy = CalculateEntropy(initDataset.size(), c, perSize);
  double cost = (time + float(kRate * space)) / entropy;

  if (cost <= *optimalCost) {
    *optimalCost = cost;
    rootStruct->rootChildNum = c;
    rootStruct->rootType = type;
  }
}

/**
 * @brief traverse all possible settings to find the optimal root
 * @return the type and childNumber of the optimal root
 */
RootStruct CARMI::ChooseRoot() {
  double OptimalValue = DBL_MAX;
  RootStruct rootStruct = RootStruct(0, 0);
  for (int c = 2; c <= initDataset.size() * 10; c *= 2) {
    CheckRoot<LRType, LinearRegression>(c, 0, LRRootTime, &OptimalValue,
                                        &rootStruct);
    CheckRoot<PLRType, PiecewiseLR>(c, 1, PLRRootTime, &OptimalValue,
                                    &rootStruct);
    CheckRoot<HisType, HistogramModel>(c, 2, HisRootTime, &OptimalValue,
                                       &rootStruct);
    CheckRoot<BSType, BinarySearchModel>(c, 3, CostBSTime * log(c) / log(2),
                                         &OptimalValue, &rootStruct);
  }
#ifdef DEBUG
  cout << "Best type is: " << rootStruct.rootType
       << "\tbest childNumber: " << rootStruct.rootChildNum
       << "\tOptimal Value: " << OptimalValue << endl;
#endif
  return rootStruct;
}

/**
 * @brief construct the root
 * @param rootStruct the optimal setting of root
 * @param subDataset the left and size of data points in each child node
 * @param range the range of data points in this node (root:0-size)
 * @return the constructed root
 */
// TYPE
template <typename TYPE, typename ModelType>
TYPE CARMI::ConstructRoot(const RootStruct &rootStruct, const IndexPair &range,
                          SubDataset *subDataset, int *childLeft) {
  TYPE root = TYPE(rootStruct.rootChildNum);
  *childLeft = allocateChildMemory(rootStruct.rootChildNum);
  root.model.Train(initDataset, rootStruct.rootChildNum);

  NodePartition<ModelType>(root.model, range.initRange, initDataset,
                           &(subDataset->subInit));
  NodePartition<ModelType>(root.model, range.insertRange, insertQuery,
                           &(subDataset->subInsert));
  return root;
}

/**
 * @brief store the optimal root into CARMI
 * @param rootStruct the optimal setting of root
 * @param nodeCost the cost of the index
 * @return the range of data points in all child node of the root node
 */
SubDataset CARMI::StoreRoot(const RootStruct &rootStruct, NodeCost *nodeCost) {
  IndexPair dataRange({0, int(initDataset.size())}, {0, int(findQuery.size())},
                      {0, int(insertQuery.size())});
  SubDataset subDataset = SubDataset(rootStruct.rootChildNum);
  int childLeft;
  switch (rootStruct.rootType) {
    case 0: {
      nodeCost->time = LRRootTime;
      nodeCost->space += sizeof(LRType);
      root.lrRoot = ConstructRoot<LRType, LinearRegression>(
          rootStruct, dataRange, &subDataset, &childLeft);
      break;
    }
    case 1: {
      nodeCost->time = PLRRootTime;
      nodeCost->space += sizeof(PLRType);
      root.plrRoot = ConstructRoot<PLRType, PiecewiseLR>(
          rootStruct, dataRange, &subDataset, &childLeft);
      break;
    }
    case 2: {
      nodeCost->time = HisRootTime;
      nodeCost->space += sizeof(HisType);
      root.hisRoot = ConstructRoot<HisType, HistogramModel>(
          rootStruct, dataRange, &subDataset, &childLeft);
      break;
    }
    case 3: {
      nodeCost->time = CostBSTime * log(rootStruct.rootChildNum) / log(2);
      nodeCost->space += sizeof(BSType);
      root.bsRoot = ConstructRoot<BSType, BinarySearchModel>(
          rootStruct, dataRange, &subDataset, &childLeft);
      break;
    }
  }
  return subDataset;
}
#endif  // !CONSTRUCT_ROOT_H