/**
 * @file minor_function.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_CONSTRUCT_MINOR_FUNCTION_H_
#define SRC_CARMI_CONSTRUCT_MINOR_FUNCTION_H_
#include <utility>
#include <vector>

#include "../carmi.h"
/**
 * @brief calculate the entropy of this node
 *
 * @param size the size of the entire data points
 * @param childNum the child number of this node
 * @param perSize the size of each child
 * @return double entropy
 */
double CARMI::CalculateEntropy(int size, int childNum,
                               const std::vector<IndexPair> &perSize) const {
  double entropy = 0.0;
  for (int i = 0; i < childNum; i++) {
    auto p = static_cast<float>(perSize[i].size) / size;
    if (p != 0) entropy += p * (-log2(p));
  }
  return entropy;
}

/**
 * @brief use this node to split the data points
 *
 * @tparam TYPE the type of this node
 * @param node used to split dataset
 * @param range the left and size of these data points
 * @param dataset partitioned dataset
 * @param subData the left and size of each sub dataset after being split
 */
template <typename TYPE>
void CARMI::NodePartition(const TYPE &node, const IndexPair &range,
                          const DataVectorType &dataset,
                          std::vector<IndexPair> *subData) const {
  int end = range.left + range.size;
  for (int i = range.left; i < end; i++) {
    int p = node.Predict(dataset[i].first);
    if ((*subData)[p].left == -1) {
      (*subData)[p].left = i;
    }
    (*subData)[p].size++;
  }
}

/**
 * @brief train the given node and use it to divide initDataset, trainFindQuery
 *        and trainTestQuery
 *
 * @tparam TYPE the type of this node
 * @param c the child number of this node
 * @param range the left and size of the data points
 * @param subDataset the left and size of each sub dataset after being split
 * @return TYPE node
 */
template <typename TYPE>
TYPE CARMI::InnerDivideAll(int c, const DataRange &range,
                           SubDataset *subDataset) {
  TYPE node;
  node.SetChildNumber(c);
  Train(range.initRange.left, range.initRange.size, &node);

  NodePartition<TYPE>(node, range.initRange, initDataset,
                      &(subDataset->subInit));
  subDataset->subFind = subDataset->subInit;
  NodePartition<TYPE>(node, range.insertRange, insertQuery,
                      &(subDataset->subInsert));
  return node;
}

/**
 * @brief update the previousLeaf and nextLeaf of each leaf nodes
 *
 */
void CARMI::UpdateLeaf() {
  if (kPrimaryIndex) return;
  auto it = scanLeaf.begin();
  int pre = it->second;
  auto next = it;
  next++;
  entireChild[it->second].array.nextLeaf = next->second;
  it++;

  for (; it != scanLeaf.end(); it++) {
    next = it;
    next++;
    if (next == scanLeaf.end()) {
      entireChild[it->second].array.previousLeaf = pre;
    } else {
      entireChild[it->second].array.previousLeaf = pre;
      pre = it->second;
      entireChild[it->second].array.nextLeaf = next->second;
    }
  }
  scanLeaf.clear();
}
#endif  // SRC_CARMI_CONSTRUCT_MINOR_FUNCTION_H_
