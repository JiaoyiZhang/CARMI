/**
 * @file carmi.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_CARMI_H_
#define SRC_INCLUDE_CARMI_H_

#include <float.h>

#include <map>
#include <utility>
#include <vector>

#include "./params.h"
#include "baseNode.h"
#include "construct/structures.h"
#include "dataManager/empty_block.h"

template <typename KeyType, typename ValueType>
class CARMI {
 public:
  typedef std::pair<KeyType, ValueType> DataType;
  typedef std::vector<DataType> DataVectorType;

 public:
  CARMI() {}

  /**
   * @brief Construct a new CARMI object for carmi_common
   *
   * @param initData the dataset used to initialize carmi
   * @param findData the read query
   * @param insertData the write query
   * @param insertIndex the index of each write query
   * @param lambda lambda
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
   * @param lambda lambda
   * @param record_number the number of the records
   * @param record_len the length of a record (byte)
   */
  CARMI(const void *dataset, DataVectorType &initData, DataVectorType &findData,
        DataVectorType &insertData, std::vector<int> &insertIndex,
        double lambda, int record_number, int record_len);

  // main functions
 public:
  /**
   * @brief find a record of the given key
   *
   * @param key
   * @return CARMI<KeyType, ValueType>::DataType of a node
   */
  BaseNode *Find(KeyType key, int *currslot);

  /**
   * @brief insert a data point
   *
   * @param data
   * @return true if the insertion is successful
   * @return false if the operation fails
   */
  bool Insert(DataType data);

  /**
   * @brief update a record of the given key
   *
   * @param data
   * @return true if the operation succeeds
   * @return false if the operation fails
   */
  bool Update(DataType data);

  /**
   * @brief delete the record of the given key
   *
   * @param key
   * @return true deletion is successful
   * @return false the operation fails
   */
  bool Delete(KeyType key);

  /**
   * @brief calculate the space of CARMI
   *
   * @return long double the space
   */
  long double CalculateSpace() const;

  /**
   * @brief print the structure of CARMI
   *
   * @param level the current level
   * @param type the type of root node
   * @param idx  the index of the node
   * @param levelVec used to record the level of CARMI
   * @param nodeVec used to record the number of each type of CARMI's node
   */
  void PrintStructure(int level, NodeType type, int idx,
                      std::vector<int> *levelVec,
                      std::vector<int> *nodeVec) const;

  // construction algorithms
  // main function

  /**
   * @brief main function of construction
   * @param initData the dataset used to initialize the index
   * @param findData the find queries used to training CARMI
   * @param insertData the insert queries used to training CARMI
   */
  void Construction(const DataVectorType &initData,
                    const DataVectorType &findData,
                    const DataVectorType &insertData);

 private:
  /**
   * @brief initialize entireData
   *
   * @param size the size of data points
   */
  void InitEntireData(int size);

  /**
   * @brief initialize entireChild
   *
   * @param size the size of datasets
   */
  void InitEntireChild(int size);

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
   * @param childLeft the start index of child nodes
   *
   * @return TYPE return the constructed root
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
   * @param nodeCost the space, time, cost of the index (is added ...)
   */
  void ConstructSubTree(const RootStruct &rootStruct,
                        const SubDataset &subDataset, NodeCost *nodeCost);

 private:
  /**
   * @brief the dynamic programming algorithm
   *
   * @param range the range of data points
   * @return NodeCost the cost of the subtree
   */
  NodeCost DP(const DataRange &range);

  /**
   * @brief traverse all possible settings to find the optimal inner node
   *
   * @param dataRange the range of data points in this node
   * @return NodeCost the optimal cost of this subtree
   */
  NodeCost DPInner(const DataRange &dataRange);

  /**
   * @brief traverse all possible settings to find the optimal leaf node
   *
   * @param dataRange the range of data points in this node
   * @return NodeCost the optimal cost of this subtree
   */
  NodeCost DPLeaf(const DataRange &dataRange);

  /**
   * @brief determine whether the current inner node's setting is
   *        better than the previous optimal setting
   *
   * @tparam TYPE the type of this inner node
   * @param c the child number of this inner node
   * @param type the type of this inne node
   * @param frequency_weight the frequency weight of these queries
   * @param time_cost the time cost of this inner node
   * @param dataRange the range of data points in this node
   * @param optimalCost the optimal cost of the previous setting
   * @param optimal_node_struct the optimal setting
   */
  template <typename TYPE>
  void ChooseBetterInner(int c, NodeType type, double frequency_weight,
                         double time_cost, const DataRange &dataRange,
                         NodeCost *optimalCost, TYPE *optimal_node_struct);

  /**
   * @brief calculate the time cost of find queries
   *
   * @tparam TYPE the type of this leaf node
   * @param actualSize the capacity of this leaf node
   * @param density the density of of this leaf node (array: 1)
   * @param node this leaf node
   * @param range the range of find queries
   * @return double the time cost of this leaf node
   */
  template <typename TYPE>
  double CalLeafFindTime(int actualSize, double density, const TYPE &node,
                         const IndexPair &range) const;

  /**
   * @brief calculate the time cost of insert queries
   *
   * @tparam TYPE the type of this leaf node
   * @param actualSize the capacity of this leaf node
   * @param density the density of of this leaf node (array: 1)
   * @param node this leaf node
   * @param range the range of insert queries
   * @param findRange the range of find queries
   * @return double the time cost of this leaf node
   */
  template <typename TYPE>
  double CalLeafInsertTime(int actualSize, double density, const TYPE &node,
                           const IndexPair &range,
                           const IndexPair &findRange) const;

 private:
  // greedy algorithm

  /**
   * @brief choose the optimal setting
   *
   * @tparam TYPE the type of this node
   * @param c the number of child nodes
   * @param type the type index of this node
   * @param frequency_weight the frequency weight of these queries
   * @param time_cost the time cost of this node
   * @param range the range of queries
   * @param optimal_node_struct the optimal setting
   * @param optimalCost the optimal cost
   */
  template <typename TYPE>
  void IsBetterGreedy(int c, NodeType type, double frequency_weight,
                      double time_cost, const IndexPair &range,
                      TYPE *optimal_node_struct, NodeCost *optimalCost);

  /**
   * @brief the greedy algorithm
   *
   * @param dataRange the range of these queries
   * @return NodeCost the optimal cost of the subtree
   */
  NodeCost GreedyAlgorithm(const DataRange &range);

 private:
  // store nodes

  /**
   * @brief store an inner node
   *
   * @tparam TYPE the type of this node
   * @param range the left and size of the data points in initDataset
   * @return TYPE trained node
   */
  template <typename TYPE>
  TYPE StoreInnerNode(const IndexPair &range, TYPE *node);

  /**
   * @brief store nodes
   *
   * @param storeIdx the index of this node being stored in entireChild
   * @param optimalType the type of this node
   * @param range the left and size of the data points in initDataset
   */
  void StoreOptimalNode(int storeIdx, const DataRange &range);

 private:
  // manage entireData and entireChild

  /**
   * @brief allocate a block to the current leaf node
   *
   * @param size the size of the leaf node needs to be allocated
   * @return int return idx (if it fails, return -1)
   */
  int AllocateMemory(int size);

  /**
   * @brief release the specified space
   *
   * @param left the left index
   * @param size the size
   */
  void ReleaseMemory(int left, int size);

  /**
   * @brief find the corresponding index in emptyBlocks
   *
   * @param size the size of the data points
   * @return int the index in emptyBlocks
   */
  int GetIndex(int size);

  /**
   * @brief find the actual size in emptyBlocks
   *
   * @param size the size of the data points
   * @return int the index in emptyBlocks
   */
  int GetActualSize(int size);

  /**
   * @brief allocate empty blocks into emptyBlocks[i]
   *
   * @param left the beginning idx of empty blocks
   * @param len the length of the blocks
   * @return true allocation is successful
   * @return false fails to allocate all blocks
   */
  bool AllocateEmptyBlock(int left, int len);

  /**
   * @brief allocate a block to this leaf node
   *
   * @param size the size of the leaf node needs to be allocated
   * @param idx the idx in emptyBlocks
   * @return int return idx (if it fails, return -1)
   */
  int AllocateSingleMemory(int size, int *idx);

  /**
   * @brief allocate a block to the current inner node
   *
   * @param size the size of the inner node needs to be allocated
   * @return int the starting position of the allocation, return -1, if it fails
   */
  int AllocateChildMemory(int size);

 private:
  // minor functions

  /**
   * @brief calculate the frequency weight
   *
   * @param dataRange the left and size of data points
   * @return double frequency weight
   */
  double CalculateFrequencyWeight(const DataRange &dataRange);

  /**
   * @brief calculate the entropy of this node
   *
   * @param size the size of the entire data points
   * @param childNum the child number of this node
   * @param perSize the size of each child
   * @return double entropy
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
   * @return TYPE node
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

  /**
   * @brief train the parameters of linear regression model
   *
   * @param left the start index of data points
   * @param size the size of data points
   * @param dataset
   * @param a parameter A of LR model
   * @param b parameter B of LR model
   */
  void LRTrain(const int left, const int size, const DataVectorType &dataset,
               float *a, float *b);

  /**
   * @brief extract data points (delete useless gaps and deleted data points)
   *
   * @param left the left index of the data points
   * @param size the size of the entire data points
   * @param dataset
   * @param actual the actual size of these data points
   * @return DataVectorType pure data points
   */
  DataVectorType ExtractData(const int left, const int size,
                             const DataVectorType &dataset, int *actual);

  /**
   * @brief set the y of each data point as a precentage of
   * the entire dataset size (index / size),
   * prepare for the linear regression
   *
   * @param left the left index of the data points
   * @param size the size of the entire data points
   * @param dataset
   * @return DataVectorType dataset used for training
   */
  DataVectorType SetY(const int left, const int size,
                      const DataVectorType &dataset);

  /**
   * @brief find the optimal error value from 0 to size
   *
   * @tparam TYPE the typename of this node
   * @param start_idx the start index of the data points
   * @param size the size of the data points
   * @param dataset
   * @param node used to predict the position of each data point
   */
  template <typename TYPE>
  void FindOptError(int start_idx, int size, const DataVectorType &dataset,
                    TYPE *node);
  inline float log2(double value) const { return log(value) / log(2); }

 private:
  // inner nodes

  /**
   * @brief train LR model
   *
   * @param left the start index of data points
   * @param size  the size of data points
   * @param dataset
   * @param lr model
   */
  void Train(int left, int size, const DataVectorType &dataset, LRModel *lr);

  /**
   * @brief train PLR model
   *
   * @param left the start index of data points
   * @param size  the size of data points
   * @param dataset
   * @param plr model
   */
  void Train(int left, int size, const DataVectorType &dataset, PLRModel *plr);

  /**
   * @brief train the histogram model
   *
   * @param left the start index of data points
   * @param size  the size of data points
   * @param dataset
   * @param his model
   */
  void Train(int left, int size, const DataVectorType &dataset, HisModel *his);

  /**
   * @brief train the bs model
   *
   * @param left the start index of data points
   * @param size  the size of data points
   * @param dataset
   * @param bs model
   */
  void Train(int left, int size, const DataVectorType &dataset, BSModel *bs);

 private:
  // leaf nodes

  /**
   * @brief initialize array node
   *
   * @param cap the capacity of this leaf node
   * @param left the start index of data points
   * @param size  the size of data points
   * @param dataset
   * @param arr leaf node
   */
  void Init(int cap, int left, int size, const DataVectorType &dataset,
            ArrayType *arr);

  /**
   * @brief initialize gapped array node
   *
   * @param cap the capacity of this leaf node
   * @param left the start index of data points
   * @param size  the size of data points
   * @param subDataset
   * @param ga leaf node
   */
  void Init(int cap, int left, int size, const DataVectorType &subDataset,
            GappedArrayType *ga);

  /**
   * @brief initialize external array node
   *
   * @param cap the capacity of this leaf node
   * @param start_idx the start index of data points
   * @param size the size of data points
   * @param dataset
   * @param ext leaf node
   */
  void Init(int cap, int start_idx, int size, const DataVectorType &dataset,
            ExternalArray *ext);

  /**
   * @brief train the array node
   *
   * @param start_idx the start index of data points
   * @param size the size of data points
   * @param dataset
   * @param arr leaf node
   */
  void Train(int start_idx, int size, const DataVectorType &dataset,
             ArrayType *arr);

  /**
   * @brief train the ga node
   *
   * @param start_idx the start index of data points
   * @param size the size of data points
   * @param dataset
   * @param ga leaf node
   */
  void Train(int start_idx, int size, const DataVectorType &dataset,
             GappedArrayType *ga);

  /**
   * @brief train the external array node
   *
   * @param start_idx the start index of data points
   * @param size the size of data points
   * @param dataset
   * @param ext leaf node
   */
  void Train(int start_idx, int size, const DataVectorType &dataset,
             ExternalArray *ext);

  /**
   * @brief store data points into the entireData
   *
   * @param cap the capacity of this leaf node
   * @param left the start index of data points
   * @param size  the size of data points
   * @param dataset
   * @param arr leaf node
   */
  void StoreData(int cap, int start_idx, int size,
                 const DataVectorType &dataset, ArrayType *arr);

  /**
   * @brief store data points into the entireData
   *
   * @param cap the capacity of this leaf node
   * @param left the start index of data points
   * @param size  the size of data points
   * @param subDataset
   * @param ga leaf node
   */
  void StoreData(int cap, int start_idx, int size,
                 const DataVectorType &dataset, GappedArrayType *ga);

 private:
  // for public functions

  /**
   * @brief search a key-value through binary search
   *
   * @param key
   * @param start
   * @param end
   * @return int the index of the key
   */
  int ArrayBinarySearch(double key, int start, int end) const;

  /**
   * @brief search a key-value through binary search in the gapped array
   *
   * @param key
   * @param start_idx
   * @param end_idx
   * @return int the idx of the first element >= key
   */
  int GABinarySearch(double key, int start_idx, int end_idx) const;

  /**
   * @brief search a key-value through binary search in the external leaf node
   *
   * @param key
   * @param start
   * @param end
   * @return int the idx of the first element >= key
   */
  int ExternalBinarySearch(double key, int start, int end) const;

  /**
   * @brief the main function of search a record in array
   *
   * @param key the key value
   * @param preIdx the predicted index of this node
   * @param error the error bound of this node
   * @param left the left index of this node in the entireData
   * @param size the size of this node
   * @return int the index of the record
   */
  int ArraySearch(double key, int preIdx, int error, int left, int size) const;

  /**
   * @brief the main function of search a record in gapped array
   *
   * @param key the key value
   * @param preIdx the predicted index of this node
   * @param error the error bound of this node
   * @param left the left index of this node in the entireData
   * @param maxIndex the max index of this node
   * @return int the index of the record
   */
  int GASearch(double key, int preIdx, int error, int left, int maxIndex) const;

  /**
   * @brief the main function of search a record in external array
   *
   * @param key the key value
   * @param preIdx the predicted index of this node
   * @param error the error bound of this node
   * @param left the left index of this node in the entireData
   * @param size the size of this node
   * @return int the index of the record
   */
  int ExternalSearch(double key, int preIdx, int error, int left,
                     int size) const;

  /**
   * @brief split the current leaf node into an inner node and several leaf
   * nodes
   *
   * @tparam TYPE the type of the current leaf node
   * @param isExternal check whether the current node is the external array
   * @param left the left index of this node in the entireData
   * @param size the size of this node
   * @param previousIdx the index of the previous leaf node
   * @param idx the index of the current leaf node
   */
  template <typename TYPE>
  void Split(bool isExternal, int left, int size, int previousIdx, int idx);

  /**
   * @brief print the root node
   *
   * @param level the current level
   * @param idx  the index of the node
   * @param levelVec used to record the level of CARMI
   * @param nodeVec used to record the number of each type of CARMI's node
   */
  void PrintRoot(int level, int idx, std::vector<int> *levelVec,
                 std::vector<int> *nodeVec) const;

  /**
   * @brief print the inner node
   *
   * @param level the current level
   * @param idx  the index of the node
   * @param levelVec used to record the level of CARMI
   * @param nodeVec used to record the number of each type of CARMI's node
   */
  void PrintInner(int level, int idx, std::vector<int> *levelVec,
                  std::vector<int> *nodeVec) const;

 public:
  std::vector<BaseNode> entireChild;

  // for carmi_common
  DataVectorType entireData;

  // for carmi_tree
  const void *external_data;
  int recordLength;

 private:
  CARMIRoot<DataVectorType, DataType> root;  // the root node
  int rootType;                              // the type of the root node
  unsigned int nowChildNumber;  // the number of inner nodes and leaf nodes

  double lambda;      // cost = time + lambda * space
  int querySize;      // the total frequency of queries
  int reservedSpace;  // the space needed to be reserved

  bool isPrimary;  // whether this carmi is a primary inde

  // for carmi_common
  unsigned int entireDataSize;  // the size of the entireData
  unsigned int nowDataSize;  // the used size of entireData to store data points
  int firstLeaf;             // the index of the first leaf node in entireChild
  std::vector<EmptyBlock> emptyBlocks;  // store the index of all empty
                                        // blocks(size: 1,2^i, 3*2^i, 4096)

  // for carmi_external
  int curr;  // the current insert index for external array

  DataVectorType initDataset;
  DataVectorType findQuery;
  DataVectorType insertQuery;
  std::vector<int> insertQueryIndex;

  std::map<IndexPair, NodeCost> COST;
  std::map<IndexPair, BaseNode> structMap;
  std::vector<int> scanLeaf;

  BaseNode emptyNode;
  static const IndexPair emptyRange;
  static const NodeCost emptyCost;

  // if size < kMaxKeyNum, this node is a leaf node
  static const int kMaxKeyNum;
  static const int kThreshold;  // used to initialize a leaf node
  static const float kDataPointSize;

  static const int kHisMaxChildNumber;  // the max number of children in his
  static const int kBSMaxChildNumber;   // the max number of children in bs
  static const int kMinChildNumber;     // the min child number of inner nodes
  static const int kInsertNewChildNumber;  // the child number of when splitting

  static const double kBaseNodeSpace;  // MB, the size of a node

  static const double kLRRootSpace;  // the space cost of lr root

 public:
  friend class LRType<DataVectorType, DataType>;

  friend class LRModel;
  friend class PLRModel;
  friend class HisModel;
  friend class BSModel;

  friend class ArrayType;
  friend class GappedArrayType;
  friend class ExternalArray;
};

template <typename KeyType, typename ValueType>
const int CARMI<KeyType, ValueType>::kMaxKeyNum = 1024;

template <typename KeyType, typename ValueType>
const int CARMI<KeyType, ValueType>::kThreshold = 2;

template <typename KeyType, typename ValueType>
const int CARMI<KeyType, ValueType>::kHisMaxChildNumber = 256;

template <typename KeyType, typename ValueType>
const int CARMI<KeyType, ValueType>::kBSMaxChildNumber = 20;

template <typename KeyType, typename ValueType>
const int CARMI<KeyType, ValueType>::kMinChildNumber = 16;

template <typename KeyType, typename ValueType>
const int CARMI<KeyType, ValueType>::kInsertNewChildNumber = 16;

template <typename KeyType, typename ValueType>
const IndexPair CARMI<KeyType, ValueType>::emptyRange = IndexPair(-1, 0);

template <typename KeyType, typename ValueType>
const NodeCost CARMI<KeyType, ValueType>::emptyCost = {0, 0, 0};

template <typename KeyType, typename ValueType>
const float CARMI<KeyType, ValueType>::kDataPointSize = sizeof(DataType) * 1.0 /
                                                        1024 / 1024;

template <typename KeyType, typename ValueType>
const double CARMI<KeyType, ValueType>::kBaseNodeSpace = 64.0 / 1024 / 1024;

template <typename KeyType, typename ValueType>
const double CARMI<KeyType, ValueType>::kLRRootSpace =
    sizeof(LRType<DataVectorType, DataType>) / 1024.0 / 1024.0;

template <typename KeyType, typename ValueType>
CARMI<KeyType, ValueType>::CARMI(DataVectorType &initData,
                                 DataVectorType &findData,
                                 DataVectorType &insertData,
                                 std::vector<int> &insertIndex, double l)

{
  isPrimary = false;
  lambda = l;
  firstLeaf = -1;
  nowDataSize = 0;

  initDataset = std::move(initData);
  findQuery = std::move(findData);
  insertQuery = std::move(insertData);
  insertQueryIndex = std::move(insertIndex);
  emptyNode.ga = GappedArrayType(kThreshold);
  reservedSpace =
      static_cast<float>(insertQuery.size()) / initDataset.size() * 4096 * 16;

  querySize = 0;
  for (int i = 0; i < findQuery.size(); i++) {
    querySize += findQuery[i].second;
  }
  for (int i = 0; i < insertQuery.size(); i++) {
    querySize += insertQuery[i].second;
  }

  InitEntireData(initDataset.size());
  InitEntireChild(initDataset.size());
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

  isPrimary = true;
  lambda = l;
  emptyNode.externalArray = ExternalArray(kThreshold);
  nowDataSize = 0;

  initDataset = std::move(initData);
  findQuery = std::move(findData);
  insertQuery = std::move(insertData);
  insertQueryIndex = std::move(insertIndex);

  reservedSpace =
      static_cast<float>(insertQuery.size()) / initDataset.size() * 4096 * 16;
  querySize = 0;
  for (int i = 0; i < findQuery.size(); i++) {
    querySize += findQuery[i].second;
  }
  for (int i = 0; i < insertQuery.size(); i++) {
    querySize += insertQuery[i].second;
  }

  InitEntireChild(initDataset.size());
}

#endif  // SRC_INCLUDE_CARMI_H_
