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
#ifndef SRC_INCLUDE_CARMI_H_
#define SRC_INCLUDE_CARMI_H_

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
 * @brief the main class for CARMI
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class CARMI {
 public:
  typedef std::pair<KeyType, ValueType> DataType;
  typedef std::vector<DataType> DataVectorType;

 public:
  CARMI();

  /**
   * @brief Construct a new CARMI object for carmi_common
   *
   * @param initData the dataset used to initialize carmi
   * @param findData the read query
   * @param insertData the write query
   * @param insertIndex the index of each write query
   * @param lambda lambda, used to tradeoff between time and space
   */
  CARMI(DataVectorType &initData, DataVectorType &findData,
        DataVectorType &insertData, std::vector<int> &insertIndex,
        double lambda);

  /**
   * @brief Construct a new CARMI object for carmi_external
   *
   * @param dataset the pointer of the dataset
   * @param initData the dataset used to initialize carmi
   * @param findData the read query
   * @param insertData the write query
   * @param insertIndex the index of each write query
   * @param lambda lambda, used to tradeoff between time and space
   * @param record_number the number of the records
   * @param record_len the length of a record (byte)
   */
  CARMI(const void *dataset, DataVectorType &initData, DataVectorType &findData,
        DataVectorType &insertData, std::vector<int> &insertIndex,
        double lambda, int record_number, int record_len);

  // main functions
 public:
  /**
   * @brief main function of construction
   */
  void Construction();

  /**
   * @brief find a record of the given key
   *
   * @param key the given key value
   * @param currblock the index of the second layer of the leaf node
   * @param currslot the index of the slot in a leaf node
   * @return BaseNode<KeyType, ValueType>*: the node responsible for the record
   */
  BaseNode<KeyType, ValueType> *Find(const KeyType &key, int *currblock,
                                     int *currslot);

  /**
   * @brief insert a data point
   *
   * @param datapoint the inserted data point
   * @retval true if the insertion is successful
   * @retval false if the operation fails
   */
  bool Insert(const DataType &datapoint);

  /**
   * @brief update a record of the given key
   *
   * @param datapoint the new data point
   * @retval true if the operation succeeds
   * @retval false if the operation fails
   */
  bool Update(const DataType &datapoint);

  /**
   * @brief delete the record of the given key
   *
   * @param key the key value of the deleted record
   * @retval true deletion is successful
   * @retval false the operation fails
   */
  bool Delete(const KeyType &key);

  /**
   * @brief calculate the space of CARMI
   *
   * @return long double: the space
   */
  long double CalculateSpace() const;

  /**
   * @brief print the structure of CARMI
   *
   * @param level the current level
   * @param type the type of root node
   * @param dataSize the size of the dataset
   * @param idx  the index of the node
   * @param levelVec used to record the level of CARMI
   * @param nodeVec used to record the number of each type of CARMI's node
   */
  void PrintStructure(int level, NodeType type, int dataSize, int idx,
                      std::vector<int> *levelVec, std::vector<int> *nodeVec);

 private:
  // root

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
  template <typename TYPE, typename ModelType>
  void IsBetterRoot(int c, NodeType type, double time_cost, double *optimalCost,
                    RootStruct *rootStruct);

  /**
   * @brief traverse all possible settings to find the optimal root
   *
   * @return the type and childNumber of the optimal root
   */
  RootStruct ChooseRoot();

  /**
   * @brief construct the root
   *
   * @tparam TYPE the type of the constructed root node
   * @tparam ModelType the type of the model in the root node
   * @param rootStruct the optimal setting of root
   * @param range the range of data points in this node (root:0-size)
   * @param subDataset the start index and size of data points in each child
   * node
   *
   * @return TYPE: the constructed root
   */
  template <typename TYPE, typename ModelType>
  TYPE ConstructRoot(const RootStruct &rootStruct, const DataRange &range,
                     SubDataset *subDataset);

  /**
   * @brief store the optimal root into CARMI
   *
   * @param rootStruct the optimal setting of root
   * @param nodeCost the cost of the index
   * @return SubDataset: the range of all child node of the root node
   */
  SubDataset StoreRoot(const RootStruct &rootStruct, NodeCost *nodeCost);

  /**
   * @brief construct each subtree using dp/greedy
   *
   * @param rootStruct the type and childNumber of root
   * @param subDataset the left and size of data points in each child node
   * @param nodeCost the space, time, cost of the index
   */
  void ConstructSubTree(const RootStruct &rootStruct,
                        const SubDataset &subDataset, NodeCost *nodeCost);

 private:
  /**
   * @brief the dynamic programming algorithm
   *
   * @param range the range of data points
   * @return NodeCost: the cost of the subtree
   */
  NodeCost DP(const DataRange &range);

  /**
   * @brief traverse all possible settings to find the optimal inner node
   *
   * @param dataRange the range of data points in this node
   * @return NodeCost: the optimal cost of this subtree
   */
  NodeCost DPInner(const DataRange &dataRange);

  /**
   * @brief traverse all possible settings to find the optimal leaf node
   *
   * @param dataRange the range of data points in this node
   * @return NodeCost: the optimal cost of this subtree
   */
  NodeCost DPLeaf(const DataRange &dataRange);

  /**
   * @brief determine whether the current inner node's setting is
   *        better than the previous optimal setting
   *
   * @tparam TYPE the type of this inner node
   * @param c the child number of this inner node
   * @param frequency_weight the frequency weight of these queries
   * @param time_cost the time cost of this inner node
   * @param dataRange the range of data points in this node
   * @param optimalCost the optimal cost of the previous setting
   * @param optimal_node_struct the optimal setting
   */
  template <typename TYPE>
  void ChooseBetterInner(int c, double frequency_weight, double time_cost,
                         const DataRange &dataRange, NodeCost *optimalCost,
                         TYPE *optimal_node_struct);

 private:
  // greedy algorithm

  /**
   * @brief choose the optimal setting
   *
   * @tparam TYPE the type of this node
   * @param c the number of child nodes
   * @param frequency_weight the frequency weight of these queries
   * @param time_cost the time cost of this node
   * @param range the range of queries
   * @param optimal_node_struct the optimal setting
   * @param optimalCost the optimal cost
   */
  template <typename TYPE>
  void IsBetterGreedy(int c, double frequency_weight, double time_cost,
                      const IndexPair &range, TYPE *optimal_node_struct,
                      NodeCost *optimalCost);

  /**
   * @brief the greedy algorithm
   *
   * @param dataRange the range of these queries
   * @return NodeCost: the optimal cost of the subtree
   */
  NodeCost GreedyAlgorithm(const DataRange &range);

 private:
  // store nodes

  /**
   * @brief store an inner node
   *
   * @tparam TYPE the type of this node
   * @param range the left and size of the data points in initDataset
   * @param node the node to be stored
   * @return TYPE: trained node
   */
  template <typename TYPE>
  TYPE StoreInnerNode(const IndexPair &range, TYPE *node);

  /**
   * @brief store nodes
   *
   * @param storeIdx the index of this node being stored in node
   * @param range the left and size of the data points in initDataset
   */
  void StoreOptimalNode(int storeIdx, const DataRange &range);

 private:
  // minor functions

  /**
   * @brief split the current leaf node into an inner node and several leaf
   * nodes
   *
   * @tparam TYPE the type of the current leaf node
   * @param isExternal check whether the current node is the external array
   * @param left the left index of this node in the data
   * @param size the size of this node
   * @param previousIdx the index of the previous leaf node
   * @param idx the index of the current leaf node
   */
  template <typename TYPE>
  void Split(bool isExternal, int left, int size, int previousIdx, int idx);

  /**
   * @brief calculate the frequency weight
   *
   * @param dataRange the left and size of data points
   * @return double: frequency weight
   */
  double CalculateFrequencyWeight(const DataRange &dataRange);

  /**
   * @brief calculate the entropy of this node
   *
   * @param size the size of the entire data points
   * @param childNum the child number of this node
   * @param perSize the size of each child
   * @return double: entropy
   */
  double CalculateEntropy(int size, int childNum,
                          const std::vector<IndexPair> &perSize) const;

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
  void NodePartition(const TYPE &node, const IndexPair &range,
                     const DataVectorType &dataset,
                     std::vector<IndexPair> *subData) const;

  /**
   * @brief train the given node and use it to divide initDataset,
   * trainFindQuery and trainTestQuery
   *
   * @tparam TYPE the type of this node
   * @param c the child number of this node
   * @param range the left and size of the data points
   * @param subDataset the left and size of each sub dataset after being split
   * @return TYPE: node
   */
  template <typename TYPE>
  TYPE InnerDivideAll(int c, const DataRange &range, SubDataset *subDataset);

  /**
   * @brief construct the empty node and insert it into map
   *
   * @param range the left and size of data points
   */
  void ConstructEmptyNode(const DataRange &range);

  /**
   * @brief update the previousLeaf and nextLeaf of each leaf nodes
   *
   */
  void UpdateLeaf();

 private:
  // leaf nodes

  /**
   * @brief calculate the cost of cf array node
   *
   * @param size the size of the data points
   * @param givenBlockNum the allocated number of data blocks
   * @return cost = time + kRate * space
   */
  double CalculateCFArrayCost(int size, int totalSize, int givenBlockNum);

  /**
   * @brief print the root node
   *
   * @param level the current level
   * @param dataSize the size of the dataset
   * @param levelVec used to record the level of CARMI
   * @param nodeVec used to record the number of each type of CARMI's node
   */
  void PrintRoot(int level, int dataSize, std::vector<int> *levelVec,
                 std::vector<int> *nodeVec);

  /**
   * @brief print the inner node
   *
   * @param level the current level
   * @param dataSize the size of the dataset
   * @param idx  the index of the node
   * @param levelVec used to record the level of CARMI
   * @param nodeVec used to record the number of each type of CARMI's node
   */
  void PrintInner(int level, int dataSize, int idx, std::vector<int> *levelVec,
                  std::vector<int> *nodeVec);

 public:
  NodeArrayStructure<KeyType, ValueType> node;  ///< used to manage all nodes
  DataArrayStructure<KeyType, ValueType> data;  ///< used to manage data points

  // for carmi_external
  const void *external_data;  ///< the location of the external data
  int recordLength;           ///< the length of a record

  // for carmi_tree
  int prefetchEnd;  ///< the last index of prefetched blocks
  double sumDepth;  ///< used to calculate the average depth

  // static const int kMaxDataBlockNum;  ///< the number of union in a leaf node
  // static const int
  //     kMaxDataPointNum;  ///< the maximum number of slots in a union
  // static const int kLeafMaxCapacity;  ///< the max capacity of a leaf node

 private:
  CARMIRoot<DataVectorType, KeyType> root;  ///< the root node
  int rootType;                             ///< the type of the root node

  double lambda;      ///< cost = time + lambda * space
  int querySize;      ///< the total frequency of queries
  int reservedSpace;  ///< the space needed to be reserved

  bool isPrimary;   ///< whether this carmi is a primary inde
  bool isInitMode;  ///< whether the current carmi is initial

  // for carmi_common
  int firstLeaf;  ///< the index of the first leaf node in node

  /// store the index of all empty,blocks(size: 1,2^i, 3*2^i, 4096)
  std::vector<EmptyBlock> emptyBlocks;

  // for carmi_external
  int curr;  ///< the current insert index for external array

  DataVectorType initDataset;
  DataVectorType findQuery;
  DataVectorType insertQuery;
  std::vector<int> insertQueryIndex;

  std::map<IndexPair, NodeCost> COST;
  std::map<IndexPair, BaseNode<KeyType, ValueType>> structMap;
  std::vector<int> scanLeaf;

  std::vector<std::pair<double, double>>
      prefetchData;  // <leaf node id, block number>
  std::vector<int> remainingNode;
  std::vector<DataRange> remainingRange;
  std::vector<int> prefetchNode;
  std::vector<DataRange> prefetchRange;

  BaseNode<KeyType, ValueType> emptyNode;
  static constexpr IndexPair emptyRange = {-1, 0};
  static constexpr NodeCost emptyCost = {0, 0, 0};

  static constexpr float kDataPointSize = sizeof(DataType) * 1.0 / 1024 / 1024;
  static constexpr double kBaseNodeSpace =
      64.0 / 1024 / 1024;  // MB, the size of a node
  static constexpr double kPLRRootSpace =
      sizeof(PLRType<DataVectorType, KeyType>) / 1024.0 /
      1024.0;  // the space cost of lr root

  static constexpr int kHisMaxChildNumber = 256;  // the max number  in his
  static constexpr int kBSMaxChildNumber = 16;    // the max number  in bs
  static constexpr int kMinChildNumber = 16;  // the min childnum of inner nodes
  static constexpr int kInsertNewChildNumber = 16;  // child number of splitting
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
  data.InitDataArray(1000);
}

template <typename KeyType, typename ValueType>
CARMI<KeyType, ValueType>::CARMI(DataVectorType &initData,
                                 DataVectorType &findData,
                                 DataVectorType &insertData,
                                 std::vector<int> &insertIndex, double l)

{
  isPrimary = false;
  lambda = l;
  firstLeaf = -1;
  sumDepth = 0;
  isInitMode = true;
  prefetchEnd = -1;

  initDataset = std::move(initData);
  findQuery = std::move(findData);
  insertQuery = std::move(insertData);
  insertQueryIndex = std::move(insertIndex);
  emptyNode.cfArray = CFArrayType<KeyType, ValueType>();
  reservedSpace = insertQuery.size() * 1.0 / initDataset.size() * 4096 * 16;

  querySize = 0;
  for (int i = 0; i < static_cast<int>(findQuery.size()); i++) {
    querySize += findQuery[i].second;
  }
  for (int i = 0; i < static_cast<int>(insertQuery.size()); i++) {
    querySize += insertQuery[i].second;
  }

  data.InitDataArray(initDataset.size());
}

template <typename KeyType, typename ValueType>
CARMI<KeyType, ValueType>::CARMI(const void *dataset, DataVectorType &initData,
                                 DataVectorType &findData,
                                 DataVectorType &insertData,
                                 std::vector<int> &insertIndex, double l,
                                 int record_number, int record_len)

{
  external_data = dataset;
  recordLength = record_len;
  curr = record_number;
  sumDepth = 0;
  isInitMode = true;
  prefetchEnd = -1;

  isPrimary = true;
  lambda = l;
  emptyNode.externalArray = ExternalArray<KeyType>();

  initDataset = std::move(initData);
  findQuery = std::move(findData);
  insertQuery = std::move(insertData);
  insertQueryIndex = std::move(insertIndex);

  reservedSpace =
      static_cast<float>(insertQuery.size()) / initDataset.size() * 4096 * 16;
  querySize = 0;
  for (int i = 0; i < static_cast<int>(findQuery.size()); i++) {
    querySize += findQuery[i].second;
  }
  for (int i = 0; i < static_cast<int>(insertQuery.size()); i++) {
    querySize += insertQuery[i].second;
  }
}

#endif  // SRC_INCLUDE_CARMI_H_
