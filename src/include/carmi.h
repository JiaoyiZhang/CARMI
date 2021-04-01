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

#include "../params.h"
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
  CARMI(const DataVectorType &dataset, int childNum, int kInnerID,
        int kLeafID);  // for static structure
  CARMI(const DataVectorType &initData, const DataVectorType &findData,
        const DataVectorType &insertData, const std::vector<int> &insertIndex,
        double rate, int thre);
  CARMI &operator=(const CARMI &p) {
    firstLeaf = p.firstLeaf;
    emptyNode.ga = p.emptyNode.ga;
    kAlgorithmThreshold = p.kAlgorithmThreshold;
    kRate = p.kRate;
    nowDataSize = p.nowDataSize;
    initDataset = p.initDataset;
    findQuery = p.findQuery;
    insertQuery = p.insertQuery;
    insertQueryIndex = p.insertQueryIndex;
    querySize = p.querySize;
    entireChild = p.entireChild;
    return *this;
  }

  // main functions
 public:
  BaseNode *Find(double key, int *currslot);
  bool Insert(DataType data);
  bool Update(DataType data);
  bool Delete(double key);

  long double CalculateSpace() const;
  void PrintStructure(int level, NodeType type, int idx,
                      std::vector<int> *levelVec,
                      std::vector<int> *nodeVec) const;

  // construction algorithms
  // main function
  void Construction(const DataVectorType &initData,
                    const DataVectorType &findData,
                    const DataVectorType &insertData);

  void InitEntireData(int left, int size, bool reinit);
  void InitEntireChild(int size);

 private:
  // root
  template <typename TYPE, typename ModelType>
  void IsBetterRoot(int c, NodeType type, double time_cost, double *optimalCost,
                    RootStruct *rootStruct);
  RootStruct ChooseRoot();
  template <typename TYPE, typename ModelType>
  TYPE ConstructRoot(const RootStruct &rootStruct, const DataRange &range,
                     SubDataset *subDataset);
  SubDataset StoreRoot(const RootStruct &rootStruct, NodeCost *nodeCost);

  // use the constructed root to construct lower layers
  void ConstructSubTree(const RootStruct &rootStruct,
                        const SubDataset &subDataset, NodeCost *nodeCost);

 private:
  // dynamic programming
  NodeCost DP(const DataRange &range);
  NodeCost DPInner(const DataRange &dataRange);
  NodeCost DPLeaf(const DataRange &dataRange);
  template <typename TYPE>
  void ChooseBetterInner(int c, NodeType type, double frequency_weight,
                         double time_cost, const DataRange &dataRange,
                         NodeCost *optimalCost, TYPE *optimal_node_struct);
  template <typename TYPE>
  double CalLeafFindTime(int actualSize, double density, const TYPE &node,
                         const IndexPair &range) const;
  template <typename TYPE>
  double CalLeafInsertTime(int actualSize, double density, const TYPE &node,
                           const IndexPair &range,
                           const IndexPair &findRange) const;

 private:
  // greedy algorithm
  template <typename TYPE>
  void IsBetterGreedy(int c, NodeType type, double frequency_weight,
                      double time_cost, const IndexPair &range,
                      TYPE *optimal_node_struct, NodeCost *optimalCost);
  NodeCost GreedyAlgorithm(const DataRange &range);

 private:
  // store nodes
  template <typename TYPE>
  TYPE StoreInnerNode(const IndexPair &range, TYPE *node);
  void StoreOptimalNode(int storeIdx, const DataRange &range);

 private:
  // manage entireData and entireChild
  int AllocateMemory(int size);
  void ReleaseMemory(int left, int size);

  int GetIndex(int size);
  int GetActualSize(int size);
  bool AllocateEmptyBlock(int left, int len);
  int AllocateSingleMemory(int size, int *idx);
  int AllocateChildMemory(int size);

 private:
  // minor functions
  double CalculateFrequencyWeight(const DataRange &dataRange);
  double CalculateEntropy(int size, int childNum,
                          const std::vector<IndexPair> &perSize) const;
  template <typename TYPE>
  void NodePartition(const TYPE &node, const IndexPair &range,
                     const DataVectorType &dataset,
                     std::vector<IndexPair> *subData) const;
  template <typename TYPE>
  TYPE InnerDivideAll(int c, const DataRange &range, SubDataset *subDataset);
  void ConstructEmptyNode(const DataRange &range);
  void UpdateLeaf();
  void LRTrain(const int left, const int size, const DataVectorType &dataset,
               float *a, float *b);
  DataVectorType ExtractData(const int left, const int size,
                             const DataVectorType &dataset, int *actual);
  DataVectorType SetY(const int left, const int size,
                      const DataVectorType &dataset);
  template <typename TYPE>
  void FindOptError(int start_idx, int size, const DataVectorType &dataset,
                    TYPE *node);
  inline float log2(double value) const { return log(value) / log(2); }

 private:
  // inner nodes
  void InitLR(const DataVectorType &dataset, LRModel *lr);
  void InitPLR(const DataVectorType &dataset, PLRModel *plr);
  void InitHis(const DataVectorType &dataset, HisModel *his);
  void InitBS(const DataVectorType &dataset, BSModel *bs);
  void Train(int left, int size, const DataVectorType &dataset, LRModel *lr);
  void Train(int left, int size, const DataVectorType &dataset, PLRModel *plr);
  void Train(int left, int size, const DataVectorType &dataset, HisModel *his);
  void Train(int left, int size, const DataVectorType &dataset, BSModel *bs);

 private:
  // leaf nodes
  void Init(int cap, int left, int size, const DataVectorType &dataset,
            ArrayType *arr);
  void Init(int cap, int left, int size, const DataVectorType &subDataset,
            GappedArrayType *ga);
  void Init(int cap, int start_idx, int size, const DataVectorType &dataset,
            ExternalArray *ext);
  void Train(int start_idx, int size, const DataVectorType &dataset,
             ArrayType *arr);
  void Train(int start_idx, int size, const DataVectorType &dataset,
             GappedArrayType *ga);
  void Train(int start_idx, int size, const DataVectorType &dataset,
             ExternalArray *ext);
  void StoreData(int cap, int start_idx, int size,
                 const DataVectorType &dataset, ArrayType *arr);
  void StoreData(int cap, int start_idx, int size,
                 const DataVectorType &dataset, GappedArrayType *ga);

 private:
  // for public functions
  int ArrayBinarySearch(double key, int start, int end) const;
  int GABinarySearch(double key, int start_idx, int end_idx) const;
  int ExternalBinarySearch(double key, int start, int end) const;
  int ArraySearch(double key, int preIdx, int error, int left, int size) const;
  int GASearch(double key, int preIdx, int error, int left, int maxIndex) const;
  int ExternalSearch(double key, int preIdx, int error, int left,
                     int size) const;

  template <typename TYPE>
  void Split(bool isExternal, int left, int size, int previousIdx, int idx);

  void PrintRoot(int level, int idx, std::vector<int> *levelVec,
                 std::vector<int> *nodeVec) const;
  void PrintInner(int level, int idx, std::vector<int> *levelVec,
                  std::vector<int> *nodeVec) const;

 private:
  // for static RMI
  template <typename TYPE>
  void InitSRMILeaf(const IndexPair &range, TYPE *node);
  template <typename ROOTTYPE, typename ROOTMODEL, typename INNERTYPE>
  ROOTTYPE InitSRMIRoot(int childNum, const IndexPair &range);

 public:
  CARMIRoot root;
  int rootType;
  std::vector<BaseNode> entireChild;
  DataVectorType entireData;
  KeyType *external_data;
  int kRecordLen;

  int curr;  // the current insert index for external array

 public:
  bool kIsWriteHeavy;
  int firstLeaf;
  unsigned int nowDataSize;
  unsigned int entireDataSize;

 private:
  unsigned int entireChildNumber;
  unsigned int nowChildNumber;
  std::vector<EmptyBlock> emptyBlocks;

  DataVectorType initDataset;
  DataVectorType findQuery;
  DataVectorType insertQuery;
  std::vector<int> insertQueryIndex;

  std::map<IndexPair, NodeCost> COST;
  std::map<IndexPair, BaseNode> structMap;
  std::vector<int> scanLeaf;
  BaseNode emptyNode;
  IndexPair emptyRange = IndexPair(-1, 0);

  const NodeCost emptyCost = {0, 0, 0};
  const int kMaxKeyNum = 1024;
  const int kThreshold = 2;  // used to initialize a leaf node

  int querySize;
  double kRate;
  int kAlgorithmThreshold;

  int kLeafNodeID;   // for static structure
  int kInnerNodeID;  // for static structure

  const double kLRRootSpace = sizeof(LRType) / 1024.0 / 1024.0;
  const double kPLRRootSpace = sizeof(PLRType) / 1024.0 / 1024.0;
  const double kHisRootSpace = sizeof(HisType) / 1024.0 / 1024.0;
  const double kBSRootSpace = sizeof(BSType) / 1024.0 / 1024.0;

 public:
  friend class LRType;
  friend class PLRType;
  friend class HisType;
  friend class BSType;

  friend class LRModel;
  friend class PLRModel;
  friend class HisModel;
  friend class BSModel;

  friend class ArrayType;
  friend class GappedArrayType;
  friend class ExternalArray;
};

template <typename KeyType, typename ValueType>
CARMI<KeyType, ValueType>::CARMI(const DataVectorType &initData,
                                 const DataVectorType &findData,
                                 const DataVectorType &insertData,
                                 const std::vector<int> &insertIndex,
                                 double rate, int thre) {
  firstLeaf = -1;
  emptyNode.ga = GappedArrayType(kThreshold);
  kAlgorithmThreshold = thre;
  kRate = rate;
  nowDataSize = 0;
  initDataset = initData;
  findQuery = findData;
  insertQuery = insertData;
  insertQueryIndex = insertIndex;
  querySize = 0;
  for (int i = 0; i < findData.size(); i++) {
    querySize += findData[i].second;
  }
  for (int i = 0; i < insertData.size(); i++) {
    querySize += insertData[i].second;
  }

  InitEntireChild(initDataset.size());
}
#endif  // SRC_INCLUDE_CARMI_H_
