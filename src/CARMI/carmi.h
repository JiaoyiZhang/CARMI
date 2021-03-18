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
#ifndef SRC_CARMI_CARMI_H_
#define SRC_CARMI_CARMI_H_

#include <float.h>

#include <map>
#include <utility>
#include <vector>

#include "../params.h"
#include "baseNode.h"
#include "construct/structures.h"
#include "dataManager/empty_block.h"

class CARMI {
 public:
  CARMI(const DataVectorType &dataset, int childNum, int kInnerID,
        int kLeafID);  // RMI
  CARMI(const DataVectorType &initData, const DataVectorType &findData,
        const DataVectorType &insertData, const std::vector<int> &insertIndex,
        double rate, int thre) {
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

    if (!kPrimaryIndex)
      InitEntireData(0, initDataset.size(), false);
    else
      externalData = initDataset;
    InitEntireChild(initDataset.size());
    Construction(initData, findData, insertData);
  }
  explicit CARMI(const DataVectorType &initData) {
    nowDataSize = 0;
    initDataset = initData;
    DataVectorType tmp;
    querySize = initData.size();
    root.hisRoot = HisType(131072);

    if (!kPrimaryIndex)
      InitEntireData(0, initDataset.size(), false);
    else
      externalData = initDataset;
    InitEntireChild(initDataset.size());
    Construction(initData, tmp, tmp);
  }
  class iterator {
   public:
    inline iterator() : currnode(NULL), currslot(0) {}
    inline iterator(CARMI *t, BaseNode *l, int s)
        : tree(t), currnode(l), currslot(s) {}
    inline const double &key() const {
      int left;
      if (kPrimaryIndex) {
        left = currnode->externalArray.m_left;
        return tree->externalData[left + currslot].first;
      } else {
        left = currnode->array.m_left;
        return tree->entireData[left + currslot].first;
      }
    }
    inline const double &data() const {
      int left;
      if (kPrimaryIndex) {
        left = currnode->externalArray.m_left;
        return tree->externalData[left + currslot].second;
      } else {
        left = currnode->array.m_left;
        return tree->entireData[left + currslot].second;
      }
    }
    inline iterator &end() const {
      static iterator it;
      it.currnode = NULL;
      it.currslot = -1;
      return it;
    }
    inline bool operator==(const iterator &x) const {
      return (x.currnode == currnode) && (x.currslot == currslot);
    }
    inline bool operator!=(const iterator &x) const {
      return (x.currnode != currnode) || (x.currslot != currslot);
    }

    inline iterator &operator++() {
      if (!kPrimaryIndex) {
        int left = currnode->array.m_left;
        while (currslot < (currnode->array.flagNumber & 0x00FFFFFF) ||
               currnode->array.nextLeaf != -1) {
          currslot++;
          if (tree->entireData[left + currslot].first != DBL_MIN) return *this;
          if (currslot == (currnode->array.flagNumber & 0x00FFFFFF))
            currnode = &(tree->entireChild[currnode->array.nextLeaf]);
        }
        return end();
      } else {
        int left = currnode->externalArray.m_left;
        if (tree->externalData[left + currslot].first != DBL_MIN) {
          currslot++;
          return *this;
        } else {
          return end();
        }
      }
    }

   private:
    CARMI *tree;
    BaseNode *currnode;
    int currslot;
  };
  long double CalculateSpace() const;
  void PrintStructure(int level, NodeType type, int idx,
                      std::vector<int> *levelVec,
                      std::vector<int> *nodeVec) const;

  // pair<double, double> Find(double key) const;
  iterator Find(double key);
  bool Insert(DataType data);
  bool Update(DataType data);
  bool Delete(double key);

 private:
  void Construction(const DataVectorType &initData,
                    const DataVectorType &findData,
                    const DataVectorType &insertData);
  template <typename TYPE, typename ModelType>
  void IsBetterRoot(int c, NodeType type, double time_cost, double *optimalCost,
                    RootStruct *rootStruct);
  RootStruct ChooseRoot();
  template <typename TYPE, typename ModelType>
  TYPE ConstructRoot(const RootStruct &rootStruct, const DataRange &range,
                     SubDataset *subDataset, int *childLeft);
  SubDataset StoreRoot(const RootStruct &rootStruct, NodeCost *nodeCost);

  void ConstructSubTree(const RootStruct &rootStruct,
                        const SubDataset &subDataset, NodeCost *nodeCost);

 private:
  NodeCost DP(const DataRange &range);
  double CalculateEntropy(int size, int childNum,
                          const std::vector<IndexPair> &perSize) const;
  template <typename TYPE>
  void NodePartition(const TYPE &node, const IndexPair &range,
                     const DataVectorType &dataset,
                     std::vector<IndexPair> *subData) const;
  double CalculateFrequencyWeight(const DataRange &dataRange);
  template <typename TYPE>
  TYPE InnerDivideAll(int c, const DataRange &range, SubDataset *subDataset);
  template <typename TYPE>
  void ChooseBetterInner(int c, NodeType type, double frequency_weight,
                         double time_cost, const DataRange &dataRange,
                         NodeCost *optimalCost, TYPE *optimal_node_struct);
  NodeCost DPInner(const DataRange &dataRange);

  template <typename TYPE>
  double CalLeafFindTime(int actualSize, double density, const TYPE &node,
                         const IndexPair &range) const;
  template <typename TYPE>
  double CalLeafInsertTime(int actualSize, double density, const TYPE &node,
                           const IndexPair &range,
                           const IndexPair &findRange) const;
  NodeCost DPLeaf(const DataRange &dataRange);

  template <typename TYPE>
  void IsBetterGreedy(int c, NodeType type, double frequency_weight,
                      double time_cost, const IndexPair &range,
                      TYPE *optimal_node_struct, NodeCost *optimalCost);
  NodeCost GreedyAlgorithm(const DataRange &range);

  template <typename TYPE>
  TYPE StoreInnerNode(const DataRange &range);
  void StoreOptimalNode(int storeIdx, int optimalType, const DataRange &range);
  bool AllocateEmptyBlock(int left, int len);
  int GetIndex(int size);
  void InitEntireData(int left, int size, bool reinit);
  int AllocateMemory(int size);
  void ReleaseMemory(int left, int size);

  void InitEntireChild(int size);
  int AllocateChildMemory(int size);

  void InitLR(const DataVectorType &dataset, LRModel *lr);
  void InitPLR(const DataVectorType &dataset, PLRModel *plr);
  void InitHis(const DataVectorType &dataset, HisModel *his);
  void InitBS(const DataVectorType &dataset, BSModel *bs);
  void Train(int left, int size, const DataVectorType &dataset, LRModel *lr);
  void Train(int left, int size, const DataVectorType &dataset, PLRModel *plr);
  void Train(int left, int size, const DataVectorType &dataset, HisModel *his);
  void Train(int left, int size, const DataVectorType &dataset, BSModel *bs);
  void Train(int left, int size, LRModel *lr);
  void Train(int left, int size, PLRModel *plr);
  void Train(int left, int size, HisModel *his);
  void Train(int left, int size, BSModel *bs);

  void InitArray(int cap, int left, int size, const DataVectorType &dataset,
                 ArrayType *arr);
  void UpdatePara(int cap, int size, ArrayType *arr);
  void StoreData(int start_idx, int size, const DataVectorType &dataset,
                 ArrayType *arr);
  void Train(int start_idx, int size, const DataVectorType &dataset,
             ArrayType *arr);
  void Train(int start_idx, int size, ArrayType *arr);

  void InitGA(int cap, int left, int size, const DataVectorType &subDataset,
              GappedArrayType *ga);

  void UpdatePara(int cap, int size, GappedArrayType *ga);
  void StoreData(int start_idx, int size, const DataVectorType &dataset,
                 GappedArrayType *ga);
  void Train(int start_idx, int size, const DataVectorType &dataset,
             GappedArrayType *ga);
  void Train(int start_idx, int size, GappedArrayType *ga);

  void InitExternalArray(ExternalArray *ycsb, int start_idx, int size);
  void Train(ExternalArray *ycsb, int start_idx, int size);

  void UpdateLeaf();
  int ArrayBinarySearch(double key, int start, int end) const;
  int GABinarySearch(double key, int start_idx, int end_idx) const;
  int ExternalBinarySearch(double key, int start, int end) const;
  int ArraySearch(double key, int preIdx, int error, int left, int size) const;
  int GASearch(double key, int preIdx, int error, int left, int maxIndex) const;
  int ExternalSearch(double key, int preIdx, int error, int left,
                     int size) const;

  inline float log2(double value) const { return log(value) / log(2); }

 public:
  CARMIRoot root;
  int rootType;
  std::vector<BaseNode> entireChild;
  DataVectorType entireData;
  DataVectorType externalData;

  int curr;  // the current insert index for external array

 private:
  unsigned int entireChildNumber;
  unsigned int nowChildNumber;
  unsigned int nowDataSize;
  unsigned int entireDataSize;
  std::vector<EmptyBlock> emptyBlocks;

  DataVectorType initDataset;
  DataVectorType findQuery;
  DataVectorType insertQuery;
  std::vector<int> insertQueryIndex;

  std::map<IndexPair, NodeCost> COST;
  std::map<IndexPair, BaseNode> structMap;
  std::map<double, int> scanLeaf;
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
#endif  // SRC_CARMI_CARMI_H_
