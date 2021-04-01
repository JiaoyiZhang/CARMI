/**
 * @file construct_root.h
 * @author Jiaoyi
 * @brief functions for constructing the root
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CONSTRUCT_CONSTRUCT_ROOT_H_
#define SRC_INCLUDE_CONSTRUCT_CONSTRUCT_ROOT_H_
#include <vector>

#include "../carmi.h"
#include "./dp.h"
#include "./store_node.h"
#include "./structures.h"

/**
 * @brief determine whether the current setting is
 *        better than the previous optimal setting (more details)
 *
 * @tparam TYPE the typename of this node
 * @tparam ModelType the typename of the model in this node
 * @param c the child number of this node
 * @param type the type of this node
 * @param time_cost the time cost of this node
 * @param optimalCost the previous optimal setting
 * @param rootStruct used to record the optimal setting
 */
template <typename KeyType, typename ValueType>
template <typename TYPE, typename ModelType>
void CARMI<KeyType, ValueType>::IsBetterRoot(int c, NodeType type,
                                             double time_cost,
                                             double *optimalCost,
                                             RootStruct *rootStruct) {
  std::vector<IndexPair> perSize(c, emptyRange);
  double space_cost = carmi_params::kBaseNodeSpace * c;

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
 *
 * @return the type and childNumber of the optimal root
 */
template <typename KeyType, typename ValueType>
RootStruct CARMI<KeyType, ValueType>::ChooseRoot() {
  double OptimalValue = DBL_MAX;
  RootStruct rootStruct(0, 0);
  for (int c = carmi_params::kMinChildNumber; c <= initDataset.size() * 10;
       c *= 2) {
    IsBetterRoot<LRType, LinearRegression>(
        c, LR_ROOT_NODE, carmi_params::kLRRootTime, &OptimalValue, &rootStruct);
    IsBetterRoot<PLRType, PiecewiseLR>(c, PLR_ROOT_NODE,
                                       carmi_params::kPLRRootTime,
                                       &OptimalValue, &rootStruct);
    IsBetterRoot<HisType, HistogramModel>(c, HIS_ROOT_NODE,
                                          carmi_params::kHisRootTime,
                                          &OptimalValue, &rootStruct);
    IsBetterRoot<BSType, BinarySearchModel>(c, BS_ROOT_NODE,
                                            carmi_params::kCostBSTime * log2(c),
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
 *
 * @tparam TYPE the type of the constructed root node
 * @tparam ModelType the type of the model in the root node
 * @param rootStruct the optimal setting of root
 * @param range the range of data points in this node (root:0-size)
 * @param subDataset the start index and size of data points in each child node
 * @param childLeft the start index of child nodes
 *
 * @return TYPE return the constructed root
 */
template <typename KeyType, typename ValueType>
template <typename TYPE, typename ModelType>
TYPE CARMI<KeyType, ValueType>::ConstructRoot(const RootStruct &rootStruct,
                                              const DataRange &range,
                                              SubDataset *subDataset) {
  TYPE root(rootStruct.rootChildNum);
  root.childLeft = AllocateChildMemory(rootStruct.rootChildNum);
  root.model.Train(initDataset, rootStruct.rootChildNum);

  NodePartition<ModelType>(root.model, range.initRange, initDataset,
                           &(subDataset->subInit));
  subDataset->subFind = subDataset->subInit;
  NodePartition<ModelType>(root.model, range.insertRange, insertQuery,
                           &(subDataset->subInsert));
  return root;
}

/**
 * @brief store the optimal root into CARMI
 *
 * @param rootStruct the optimal setting of root
 * @param nodeCost the cost of the index
 * @return SubDataset: the range of all child node of the root node
 */
template <typename KeyType, typename ValueType>
SubDataset CARMI<KeyType, ValueType>::StoreRoot(const RootStruct &rootStruct,
                                                NodeCost *nodeCost) {
  DataRange dataRange({0, static_cast<int>(initDataset.size())},
                      {0, static_cast<int>(findQuery.size())},
                      {0, static_cast<int>(insertQuery.size())});
  SubDataset subDataset(rootStruct.rootChildNum);
  switch (rootStruct.rootType) {
    case LR_ROOT_NODE: {
      nodeCost->time = carmi_params::kLRRootTime;
      nodeCost->space += sizeof(LRType);
      root = ConstructRoot<LRType, LinearRegression>(rootStruct, dataRange,
                                                     &subDataset);
      break;
    }
    case PLR_ROOT_NODE: {
      nodeCost->time = carmi_params::kPLRRootTime;
      nodeCost->space += sizeof(PLRType);
      root = ConstructRoot<PLRType, PiecewiseLR>(rootStruct, dataRange,
                                                 &subDataset);
      break;
    }
    case HIS_ROOT_NODE: {
      nodeCost->time = carmi_params::kHisRootTime;
      nodeCost->space += sizeof(HisType);
      root = ConstructRoot<HisType, HistogramModel>(rootStruct, dataRange,
                                                    &subDataset);
      break;
    }
    case BS_ROOT_NODE: {
      nodeCost->time =
          carmi_params::kCostBSTime * log2(rootStruct.rootChildNum);
      nodeCost->space += sizeof(BSType);
      root = ConstructRoot<BSType, BinarySearchModel>(rootStruct, dataRange,
                                                      &subDataset);
      break;
    }
  }
  return subDataset;
}
#endif  // SRC_INCLUDE_CONSTRUCT_CONSTRUCT_ROOT_H_
