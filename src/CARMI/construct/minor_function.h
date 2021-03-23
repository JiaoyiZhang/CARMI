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
#ifdef DEBUG
    if (p > subData->size()) {
      std::cout << "wrong!" << std::endl;
      node.Predict(dataset[i].first);
    }
#endif  // DEBUG
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
  Train(range.initRange.left, range.initRange.size, initDataset, &node);

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
  entireChild[scanLeaf[0]].array.nextLeaf = scanLeaf[1];
  int end = scanLeaf.size() - 1;
  entireChild[end].array.nextLeaf = -1;
  entireChild[end].array.previousLeaf = scanLeaf[end - 1];
  for (int i = 1; i < end; i++) {
    entireChild[scanLeaf[i]].array.nextLeaf = scanLeaf[i + 1];
    entireChild[scanLeaf[i]].array.previousLeaf = scanLeaf[i - 1];
  }

  scanLeaf.clear();
}

/**
 * @brief calculate the frequency weight
 *
 * @param dataRange the left and size of data points
 * @return double frequency weight
 */
double CARMI::CalculateFrequencyWeight(const DataRange &dataRange) {
  float frequency = 0.0;
  int findEnd = dataRange.findRange.left + dataRange.findRange.size;
  for (int l = dataRange.findRange.left; l < findEnd; l++)
    frequency += findQuery[l].second;
  int insertEnd = dataRange.insertRange.left + dataRange.insertRange.size;
  for (int l = dataRange.insertRange.left; l < insertEnd; l++)
    frequency += insertQuery[l].second;
  double frequency_weight = frequency / querySize;
  return frequency_weight;
}

void CARMI::ConstructEmptyNode(const DataRange &range) {
  BaseNode optimal_node_struct;
  if (kPrimaryIndex) {
    ExternalArray tmp;
    Train(range.initRange.left, range.initRange.size, initDataset, &tmp);
    optimal_node_struct.externalArray = tmp;
  } else {
    GappedArrayType tmpNode(kThreshold);
    tmpNode.density = 0.5;
    Train(range.initRange.left, range.initRange.size, initDataset, &tmpNode);
    optimal_node_struct.ga = tmpNode;
  }
  structMap.insert({range.initRange, optimal_node_struct});
}
#endif  // SRC_CARMI_CONSTRUCT_MINOR_FUNCTION_H_
