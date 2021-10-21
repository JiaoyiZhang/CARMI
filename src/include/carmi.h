/**
 * @file carmi.h
 * @author Jiaoyi
 * @brief the implementation of CARMI
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CARMI_H_
#define CARMI_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

#include "./params.h"
#include "baseNode.h"
#include "construct/structures.h"
#include "memoryLayout/data_array.h"
#include "memoryLayout/node_array.h"

#define log2(value) log(value) / log(2)

/**
 * @brief The main class of the cache-aware learned index framework, called
 * CARMI.
 *
 * CARMI uses a hybrid construction algorithm to automatically construct the
 * optimal index structures under various datasets without any manual tuning.
 *
 * This class contains the specific implementation of all the basic functions of
 * the CARMI framework, including the basic operations and construction of the
 * index, and supports two different types of CARMI: common CARMI and external
 * CARMI.
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class CARMI {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief the pair of data points
   */
  typedef std::pair<KeyType, ValueType> DataType;

  /**
   * @brief the vector of data points, which is the type of dataset
   */
  typedef std::vector<DataType> DataVectorType;

  CARMI();

  /**
   * @brief Construct a new CARMI object for CARMICommon
   *
   * @param[in] initData the init dataset, used to train models and construct
   * the index tree
   * @param[in] findData the find dataset, used as historical find queries to
   * assist in constructing index
   * @param[in] insertData the insert dataset, used as historical insert queries
   * to assist in constructing index
   * @param[in] lambda cost = time + lambda * space, used to tradeoff between
   * time and space cost
   */
  CARMI(const DataVectorType &initData, const DataVectorType &findData,
        const DataVectorType &insertData, double lambda);

  /**
   * @brief Construct a new CARMI object for CARMIExternal
   *
   * @param[in] dataset the pointer of the dataset
   * @param[in] insertData the insert dataset, used as historical insert queries
   * to assist in constructing index
   * @param[in] lambda cost = time + lambda * space, used to tradeoff between
   * time and space cost
   * @param[in] record_number the number of records
   * @param[in] record_len the length of a record
   */
  CARMI(const void *dataset, const std::vector<KeyType> &insertData,
        double lambda, int record_number, int record_len);

  /**
   * @brief main function of index construction.
   *
   * This function will be called after the specific implementation of CARMI
   * completes data preprocessing and other operations, and uses the
   * corresponding parameters to construct an index structure with good
   * performance for the given dataset.
   */
  void Construction();

 public:
  // *** Basic Functions of CARMI Objects

  /**
   * @brief find a data point of the given key value and return its position
   *
   * @param[in] key the given key value
   * @param[out] currblock the index of the data block of the returned leaf node
   * @param[out] currslot the index of the data point in the data block
   * @return BaseNode<KeyType, ValueType>*: the leaf node which manages this
   * data point
   */
  BaseNode<KeyType, ValueType> *Find(const KeyType &key, int *currblock,
                                     int *currslot);

  /**
   * @brief insert a data point
   *
   * @param[in] datapoint the inserted data point
   * @retval true if the insertion is successful
   * @retval false if the operation fails
   */
  bool Insert(const DataType &datapoint);

  /**
   * @brief update a data point of the given key
   *
   * @param[in] datapoint the data point needed to be updated
   * @retval true if the operation succeeds
   * @retval false if the operation fails
   */
  bool Update(const DataType &datapoint);

  /**
   * @brief delete the record of the given key
   *
   * @param[in] key the key value of the deleted record
   * @retval true deletion is successful
   * @retval false the operation fails
   */
  bool Delete(const KeyType &key);

 public:
  // *** Functions of Getting some information of CARMI Objects

  /**
   * @brief calculate the space cost of the CARMI object
   *
   * @return long double: the space cost (byte)
   */
  long double CalculateSpace() const;

  /**
   * @brief Get the Root Type object
   *
   * @return int the root type
   */
  int GetRootType() { return root.flagNumber; }

  /**
   * @brief return the information of the tree node
   *
   * @param[in] idx  the index of the node
   */

  /**
   * @brief Get the information of the tree node, return the type identifier of
   * this node, the number of its child nodes and the starting index of the
   * first child node in the node array.
   *
   * @param[in] idx the index of the node in the node array
   * @param[out] childNumber the number of the child nodes of this node
   * @param[out] childStartIndex the starting index of the first child node
   * @return int the type identifier of this node
   */
  int GetNodeInfo(int idx, int *childNumber, int *childStartIndex);

 private:
  //*** Private Functions of Constructing Root Nodes of CARMI Objects

  /**
   * @brief Determine whether the current setting of the root node is better
   * than the previous optimal setting
   *
   * This function calculates the total cost under the current setting according
   * to the cost model in our paper with the fixed time cost of the given root
   * node and the prediction of the model on the dataset. If the current cost is
   * less than the given previous optimal cost, then update the optimal cost and
   * optimal settings, otherwise return directly.
   *
   * @tparam RootNodeType the typename of this node
   * @param[in] c the child number of this node
   * @param[in] type the type identifier of this node
   * @param[out] optimalCost the optimal cost
   * @param[out] rootStruct the optimal cost setting
   */
  template <typename RootNodeType>
  void IsBetterRoot(int c, NodeType type, double *optimalCost,
                    RootStruct *rootStruct);

  /**
   * @brief the main function of selecting the optimal root node setting
   *
   * This function will generate many different combinations of node settings as
   * the input of the IsBetterRoot function, find the optimal root node from
   * them, and finally return the optimal root node.
   *
   * @return the type and childNumber of the optimal root
   */
  RootStruct ChooseRoot();

  /**
   * @brief The main function to store the root node
   *
   * This function constructs the root node based on the optimal root node
   * setting selected by the ChooseRoot function. It generates the corresponding
   * parameters as the input of the ConstructRoot function to construct the root
   * node and divide the dataset with the model of the root node to prepare for
   * the construction of its child nodes.
   *
   * @param[in] rootStruct the optimal setting of the root node
   * @param[out] nodeCost the cost of the root node
   * @return SubDataset:  the starting index and size of sub-dataset in each
   * child node
   */
  SubDataset StoreRoot(const RootStruct &rootStruct, NodeCost *nodeCost);

 private:
  //*** Main Private Function of Constructing Child Nodes of the Root Node

  /**
   * @brief The main function of constructing each subtree using dp/greedy
   * algorithm.
   *
   * This function recursively constructs the optimal sub-index tree for each
   * child node of the root node and its corresponding sub-dataset, and stores
   * them in the corresponding node and data arrays. At this point, since the
   * root node has already been determined, we can train the prefetch prediction
   * model of the root node. When constructing leaf nodes, we try to store data
   * points according to the output of the prefetch prediction model as much as
   * possible.
   *
   * @param[in] subDataset the range of the sub-dataset corresponding to each
   * child node
   * @param[inout] nodeCost the space cost, time cost, and the total cost of the
   * index
   */
  void ConstructSubTree(const SubDataset &subDataset, NodeCost *nodeCost);

 private:
  //*** Private Functions of Dynamic Programming Algorithm

  /**
   * @brief The main function of dynamic programming algorithm to construct the
   * optimal sub-index tree
   *
   * This function is the outermost interface of the dynamic programming
   * algorithm. It compares the size of the dataset with the parameters, and
   * chooses to construct a leaf node, an inner node, or both, and choose the
   * better one. Finally, the cost of the sub-tree corresponding to the
   * sub-dataset is returned.
   *
   * @param[in] range the range of the sub-dataset
   * @return NodeCost: the cost of the subtree
   */
  NodeCost DP(const DataRange &range);

  /**
   * @brief Traverse all possible settings to find the optimal inner node
   *
   * This function generates a variety of different inner node settings
   * according to the given sub-dataset, and then call the UpdateDPOptSetting
   * function to calculate their costs, select the optimal settings to store and
   * return the minimum cost.
   *
   * @param[in] range the range of the sub-dataset
   * @return NodeCost: the optimal cost of this subtree
   */
  NodeCost DPInner(const DataRange &dataRange);

  /**
   * @brief Traverse all possible settings to find the optimal leaf node
   *
   * This function constructs a leaf node directly as the current node and
   * return its cost.
   *
   * @param[in] range the range of the sub-dataset
   * @return NodeCost: the optimal cost of this subtree
   */
  NodeCost DPLeaf(const DataRange &dataRange);

  /**
   * @brief Determine whether the current inner node's setting is better than
   * the previous optimal setting
   *
   * This function uses the parameters passed by DPInner to construct the
   * current node and uses the cost model to calculate the cost of the subtree,
   * and returns the optimal node setting and cost. This function will
   * recursively call DP and GreedyAlgorithm functions to construct
   * sub-structures respectively.
   *
   * @tparam InnerNodeType the type of this inner node
   * @param[in] c the child number of this inner node
   * @param[in] frequency_weight the frequency weight of these queries
   * @param[in] dataRange the range of the sub-dataset
   * @param[inout] optimalCost the optimal cost
   * @param[inout] optimal_node_struct the optimal setting
   */
  template <typename InnerNodeType>
  void UpdateDPOptSetting(int c, double frequency_weight,
                          const DataRange &dataRange, NodeCost *optimalCost,
                          InnerNodeType *optimal_node_struct);

 private:
  //*** Private Functions of Greedy Node Selection Algorithm

  /**
   * @brief The main function of greedy node selection algorithm to construct
   * the optimal sub-index tree
   *
   * This function is the outermost interface of the greedy node selection
   * algorithm. It uses the local information to construct the current inner
   * node. The cost of the sub-tree corresponding to the sub-dataset is
   * returned.
   *
   * @param[in] dataRange the range of the sub-dataset
   * @return NodeCost: the cost of the subtree
   */
  NodeCost GreedyAlgorithm(const DataRange &dataRange);

  /**
   * @brief Determine whether the current inner node's setting is better than
   * the previous optimal setting
   *
   * This function uses the parameters to construct the current node and
   * calculates the cost of the subtree, and returns the optimal node setting
   * and cost.
   *
   * @tparam InnerNodeType the type of this inner node
   * @param[in] c the child number of this inner node
   * @param[in] frequency_weight the frequency weight of these queries
   * @param[in] range the range of the sub-dataset
   * @param[inout] optimal_node_struct the optimal setting
   * @param[inout] optimalCost the optimal cost
   */
  template <typename InnerNodeType>
  void UpdateGreedyOptSetting(int c, double frequency_weight,
                              const DataRange &range,
                              InnerNodeType *optimal_node_struct,
                              NodeCost *optimalCost);

 private:
  //*** Private Functions of Storing Tree Nodes

  /**
   * @brief The main function of storing the optimal tree nodes.
   *
   * This function uses the range of the initDataset to find the setting of the
   * current optimal node and recursively stores its child nodes, and then,
   * stores this node to the node array.
   *
   * @param[in] storeIdx the index of this node being stored in node
   * @param[in] range the left and size of the data points in initDataset
   */
  void StoreOptimalNode(int storeIdx, const DataRange &range);

  /**
   * @brief Divide the dataset using the model of this inner node and
   * recursively call the StoreOptimalNode function to store its child node, and
   * finally update its childLeft parameter
   *
   * @tparam InnerNodeType the type of this node
   * @param[in] range the range of the sub-dataset
   * @param[out] node the inner node to be stored
   */
  template <typename InnerNodeType>
  void StoreInnerNode(const IndexPair &range, InnerNodeType *node);

 private:
  //*** Private Minor Functions to Favour the Above Functions

  /**
   * @brief split the current leaf node into an inner node and several leaf
   * nodes
   *
   * This function will be triggered when the leaf node cannot accommodate more
   * data points. It constructs an inner node and multiple leaf nodes to manage
   * the sub-dataset, and completely replaces the previous leaf node.
   *
   * @tparam LeafNodeType the type of the current leaf node
   * @param[in] isExternal check whether the current node is the external array
   * @param[in] left the left index of the sub-dataset
   * @param[in] size the size of the sub-dataset
   * @param[in] previousIdx the index of the previous leaf node
   * @param[in] idx the index of the current leaf node
   */
  template <typename LeafNodeType>
  void Split(bool isExternal, int left, int size, int previousIdx, int idx);

  /**
   * @brief calculate the frequency weight
   *
   * @param[in] dataRange the range of data points
   * @return double: frequency weight
   */
  double CalculateFrequencyWeight(const DataRange &dataRange);

  /**
   * @brief calculate the entropy of this node
   *
   * @param[in] size the size of the entire data points
   * @param[in] childNum the child number of this node
   * @param[in] perSize the size of each child node of this node
   * @return double: entropy
   */
  double CalculateEntropy(int size, int childNum,
                          const std::vector<IndexPair> &perSize) const;

  /**
   * @brief use this node to split the data points of the given dataset
   *
   * @tparam InnerNodeType the type of this node
   * @param[in] node the current node used to split dataset
   * @param[in] range the left and size of these data points in the dataset
   * @param[in] dataset the partitioned dataset
   * @param[in] subData the left and size of each sub-dataset after being split
   */
  template <typename InnerNodeType>
  void NodePartition(const InnerNodeType &node, const IndexPair &range,
                     const DataVectorType &dataset,
                     std::vector<IndexPair> *subData) const;

  /**
   * @brief train the given inner node and use it to divide initDataset,
   * trainFindQuery and trainTestQuery
   *
   * @tparam InnerNodeType the type of this node
   * @param[in] c the child number of this node
   * @param[in] range the left and size of these data points in the three
   * datasets
   * @param[in] subData the left and size of each sub-dataset after being split
   * @return InnerNodeType: node
   */
  template <typename InnerNodeType>
  InnerNodeType InnerDivideAll(int c, const DataRange &range,
                               SubDataset *subDataset);

  /**
   * @brief construct the leaf node when the sub-dataset is empty
   *
   * @param[in] range the left and size of data points
   */
  void ConstructEmptyNode(const DataRange &range);

  /**
   * @brief update the previousLeaf and nextLeaf of each leaf node
   */
  void UpdateLeaf();

  /**
   * @brief calculate the cost of cf array node
   *
   * @param[in] size the size of the data points
   * @param[in] totalSize the size of the total dataset
   * @param[in] givenBlockNum the allocated number of data blocks
   * @return cost = time + kRate * space
   */
  double CalculateCFArrayCost(int size, int totalSize, int givenBlockNum);

 public:
  //*** Public Data Members of CARMI Objects

  /**
   * @brief used to manage all nodes
   *
   */
  NodeArrayStructure<KeyType, ValueType> node;

  /**
   * @brief designed for carmi_common objects, used to manage data points
   *
   */
  DataArrayStructure<KeyType, ValueType> data;

  /**
   * @brief designed for carmi_external objects, the location of the external
   * dataset
   *
   */
  const void *external_data;

  /**
   * @brief designed for carmi_external objects, the length of a record
   *
   */
  int recordLength;

  /**
   * @brief designed for calculating the avg depth
   *
   */
  double sumDepth;

 private:
  //*** Private Data Members of CARMI Objects

  /**
   * @brief the root node
   *
   */
  CARMIRoot<DataVectorType, KeyType> root;

  /**
   * @brief designed for carmi_common, the last index of the prefetched data
   * block
   *
   */
  int prefetchEnd;

  /**
   * @brief the parameter of the cost model: cost = time + lambda * space
   *
   */
  double lambda;

  /**
   * @brief the total frequency of queries
   *
   */
  int querySize;

  /**
   * @brief the space needed to be reserved for the future inserts
   *
   */
  int reservedSpace;

  /**
   * @brief whether this carmi is a primary inde
   *
   */
  bool isPrimary;

  /**
   * @brief the status of the construction: init mode or reconstruction mode
   *
   */
  bool isInitMode;

  /**
   * @brief designed for carmi_common objects, the index of the first leaf node
   *
   */
  int firstLeaf;

  /**
   * @brief designed for carmi_external, the current insert index for external
   * array
   *
   */
  int curr;

  /**
   * @brief the initialized dataset
   *
   */
  DataVectorType initDataset;

  /**
   * @brief the historical find queries
   *
   */
  DataVectorType findQuery;

  /**
   * @brief the historical insert queries
   *
   */
  DataVectorType insertQuery;

  /**
   * @brief the cost of different sub-datasets, used for the memorized dp
   * algorithm
   *
   */
  std::map<IndexPair, NodeCost> COST;

  /**
   * @brief the optimal nodes corresponding to the sub-datasets
   *
   */
  std::map<IndexPair, BaseNode<KeyType, ValueType>> structMap;

  /**
   * @brief the order of leaf nodes
   *
   */
  std::vector<int> scanLeaf;

  /**
   * @brief the dataset used to train the prefetch prediction model, each
   * element is <leaf node id, block number>
   */
  std::vector<std::pair<double, double>> prefetchData;

  /**
   * @brief the index of leaf nodes
   *
   */
  std::vector<int> remainingNode;

  /**
   * @brief the range of the sub-datasets whose nodes are leaf nodes
   *
   */
  std::vector<DataRange> remainingRange;

  /**
   * @brief the empty leaf node
   *
   */
  BaseNode<KeyType, ValueType> emptyNode;

 private:
  // *** Static Constant Options and Values of CARMI

  /**
   * @brief the empty range
   *
   */
  static constexpr IndexPair emptyRange = {-1, 0};

  /**
   * @brief the empty cost of space, time and total cost are all 0
   *
   */
  static constexpr NodeCost emptyCost = {0, 0, 0};

  /**
   * @brief the size of a node, in MB
   *
   */
  static constexpr double kBaseNodeSpace = 64.0 / 1024 / 1024;

  /**
   * @brief the space cost of p. lr root node
   *
   */
  static constexpr double kPLRRootSpace =
      sizeof(PLRType<DataVectorType, KeyType>) / 1024.0 / 1024.0;

  /**
   * @brief the maximum child number in histogram inner nodes
   *
   */
  static constexpr int kHisMaxChildNumber = 256;

  /**
   * @brief the maximum child number in bs inner nodes
   *
   */
  static constexpr int kBSMaxChildNumber = 16;

  /**
   * @brief the minimum child number of inner nodes
   *
   */
  static constexpr int kMinChildNumber = 16;

  /**
   * @briefthe number of new leaf nodes when splitting
   *
   */
  static constexpr int kInsertNewChildNumber = 16;
};

template <typename KeyType, typename ValueType>
CARMI<KeyType, ValueType>::CARMI() {
  isPrimary = false;
  firstLeaf = -1;
  sumDepth = 0;
  isInitMode = true;
  prefetchEnd = -1;

  emptyNode.cfArray = CFArrayType<KeyType, ValueType>();
  reservedSpace = 0;
  data = DataArrayStructure<KeyType, ValueType>(
      CFArrayType<KeyType, ValueType>::kMaxBlockNum, 1000);
}

template <typename KeyType, typename ValueType>
CARMI<KeyType, ValueType>::CARMI(const DataVectorType &initData,
                                 const DataVectorType &findData,
                                 const DataVectorType &insertData, double l) {
  isPrimary = false;
  lambda = l;
  firstLeaf = -1;
  sumDepth = 0;
  isInitMode = true;
  prefetchEnd = -1;

  initDataset = std::move(initData);
  findQuery = std::move(findData);
  insertQuery = std::move(insertData);
  emptyNode.cfArray = CFArrayType<KeyType, ValueType>();
  reservedSpace = insertQuery.size() * 1.0 / initDataset.size() * 4096 * 16;

  querySize = 0;
  for (int i = 0; i < static_cast<int>(findQuery.size()); i++) {
    querySize += findQuery[i].second;
  }
  for (int i = 0; i < static_cast<int>(insertQuery.size()); i++) {
    querySize += insertQuery[i].second;
  }

  data = DataArrayStructure<KeyType, ValueType>(
      CFArrayType<KeyType, ValueType>::kMaxBlockNum, initDataset.size());
}

template <typename KeyType, typename ValueType>
CARMI<KeyType, ValueType>::CARMI(const void *dataset,
                                 const std::vector<KeyType> &insertData,
                                 double l, int record_number, int record_len) {
  external_data = dataset;
  recordLength = record_len;
  curr = record_number;
  sumDepth = 0;
  isInitMode = true;
  prefetchEnd = -1;

  isPrimary = true;
  lambda = l;
  emptyNode.externalArray = ExternalArray<KeyType>();

  initDataset.resize(record_number);
  findQuery.resize(record_number);
  insertQuery.resize(insertData.size());

  querySize = 0;
  for (int i = 0; i < record_number; i++) {
    initDataset[i] = {*reinterpret_cast<const KeyType *>(
                          static_cast<const char *>(dataset) + i * record_len),
                      1};
    findQuery[i] = {initDataset[i].first, 1};
    querySize++;
  }

  for (int i = 0; i < static_cast<int>(insertData.size()); i++) {
    insertQuery[i] = {insertData[i], 1};
    querySize++;
  }

  reservedSpace =
      static_cast<float>(insertQuery.size()) / initDataset.size() * 4096 * 16;
}
#endif  // CARMI_H_
