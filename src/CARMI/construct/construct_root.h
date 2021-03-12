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
#ifndef SRC_CARMI_CONSTRUCT_CONSTRUCT_ROOT_H_
#define SRC_CARMI_CONSTRUCT_CONSTRUCT_ROOT_H_
#include <vector>

#include "../carmi.h"
#include "./dp.h"
#include "./store_node.h"
#include "./structures.h"

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
void CARMI::isBetterRoot(int c, NodeType type, double time_cost,
                         double *optimalCost, RootStruct *rootStruct) {
  std::vector<IndexPair> perSize(c, emptyRange);
  double space_cost = BaseNodeSpace * c;

  TYPE root(c);
  root.model.Train(initDataset, c);
  IndexPair range(0, initDataset.size());
  NodePartition<ModelType>(root.model, range, initDataset, &perSize);
  double entropy = CalculateEntropy(initDataset.size(), c, perSize);
  double cost = (time_cost + static_cast<float>(kRate * space_cost)) / entropy;

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
  RootStruct rootStruct(0, 0);
  for (int c = 2; c <= initDataset.size() * 10; c *= 2) {
    isBetterRoot<LRType, LinearRegression>(c, LR_ROOT_NODE, LRRootTime,
                                           &OptimalValue, &rootStruct);
    isBetterRoot<PLRType, PiecewiseLR>(c, PLR_ROOT_NODE, PLRRootTime,
                                       &OptimalValue, &rootStruct);
    isBetterRoot<HisType, HistogramModel>(c, HIS_ROOT_NODE, HisRootTime,
                                          &OptimalValue, &rootStruct);
    isBetterRoot<BSType, BinarySearchModel>(c, BS_ROOT_NODE,
                                            CostBSTime * log(c) / log(2),
                                            &OptimalValue, &rootStruct);
  }
#ifdef DEBUG
  std::cout << "Best type is: " << rootStruct.rootType
            << "\tbest childNumber: " << rootStruct.rootChildNum
            << "\tOptimal Value: " << OptimalValue << std::endl;
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

template <typename TYPE, typename ModelType>
TYPE CARMI::ConstructRoot(const RootStruct &rootStruct, const DataRange &range,
                          SubDataset *subDataset, int *childLeft) {
  TYPE root(rootStruct.rootChildNum);
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
  DataRange dataRange({0, static_cast<int>(initDataset.size())},
                      {0, static_cast<int>(findQuery.size())},
                      {0, static_cast<int>(insertQuery.size())});
  SubDataset subDataset(rootStruct.rootChildNum);
  int childLeft;
  switch (rootStruct.rootType) {
    case LR_ROOT_NODE: {
      nodeCost->time = LRRootTime;
      nodeCost->space += sizeof(LRType);
      root.lrRoot = ConstructRoot<LRType, LinearRegression>(
          rootStruct, dataRange, &subDataset, &childLeft);
      break;
    }
    case PLR_ROOT_NODE: {
      nodeCost->time = PLRRootTime;
      nodeCost->space += sizeof(PLRType);
      root.plrRoot = ConstructRoot<PLRType, PiecewiseLR>(
          rootStruct, dataRange, &subDataset, &childLeft);
      break;
    }
    case HIS_ROOT_NODE: {
      nodeCost->time = HisRootTime;
      nodeCost->space += sizeof(HisType);
      root.hisRoot = ConstructRoot<HisType, HistogramModel>(
          rootStruct, dataRange, &subDataset, &childLeft);
      break;
    }
    case BS_ROOT_NODE: {
      nodeCost->time = CostBSTime * log(rootStruct.rootChildNum) / log(2);
      nodeCost->space += sizeof(BSType);
      root.bsRoot = ConstructRoot<BSType, BinarySearchModel>(
          rootStruct, dataRange, &subDataset, &childLeft);
      break;
    }
  }
  return subDataset;
}
#endif  // SRC_CARMI_CONSTRUCT_CONSTRUCT_ROOT_H_
