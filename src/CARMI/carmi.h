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
#ifndef CARMI_H
#define CARMI_H

#include <float.h>

#include <map>
#include <vector>

#include "../params.h"
#include "baseNode.h"
#include "construct/structures.h"
#include "dataManager/empty_block.h"
using namespace std;

class CARMI {
 public:
  CARMI(const vector<pair<double, double>> &dataset, int childNum, int kInnerID,
        int kLeafID);  // RMI
  CARMI(const vector<pair<double, double>> &initData,
        const vector<pair<double, double>> &findData,
        const vector<pair<double, double>> &insertData, double rate, int thre) {
    kAlgorithmThreshold = thre;
    kRate = rate;
    nowDataSize = 0;
    initDataset = initData;
    findQuery = findData;
    insertQuery = insertData;
    querySize = findData.size() + insertData.size();

    if (!kPrimaryIndex)
      initEntireData(0, initDataset.size(), false);
    else
      entireData = initDataset;
    initEntireChild(initDataset.size());
    rootType = Construction(initData, findData, insertData);
  }
  CARMI(const vector<pair<double, double>> &initData) {
    nowDataSize = 0;
    initDataset = initData;
    vector<pair<double, double>> tmp;
    querySize = initData.size();

    if (!kPrimaryIndex)
      initEntireData(0, initDataset.size(), false);
    else
      entireData = initDataset;
    initEntireChild(initDataset.size());
    rootType = Construction(initData, tmp, tmp);
  }
  class iterator {
   public:
    inline iterator() : currnode(NULL), currslot(0){};
    inline iterator(CARMI *t, BaseNode *l, int s)
        : tree(t), currnode(l), currslot(s){};
    inline const double &key() const {
      int left;
      if (kPrimaryIndex)
        left = currnode->ycsbLeaf.m_left;
      else
        left = currnode->array.m_left;
      return tree->entireData[left + currslot].first;
    }
    inline const double &data() const {
      int left;
      if (kPrimaryIndex)
        left = currnode->ycsbLeaf.m_left;
      else
        left = currnode->array.m_left;
      return tree->entireData[left + currslot].second;
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
        int left = currnode->ycsbLeaf.m_left;
        if (tree->entireData[left + currslot].first != DBL_MIN) {
          currslot++;
          return *this;
        } else
          return end();
      }
    }

   private:
    CARMI *tree;
    BaseNode *currnode;
    int currslot;
  };

  void setKRate(double rate) { kRate = rate; }

  int Construction(const vector<pair<double, double>> &initData,
                   const vector<pair<double, double>> &findData,
                   const vector<pair<double, double>> &insertData);
  long double calculateSpace() const;
  void printStructure(vector<int> &levelVec, vector<int> &nodeVec, int level,
                      int type, int idx) const;

  // pair<double, double> Find(double key) const;
  iterator Find(double key);
  bool Insert(pair<double, double> data);
  bool Update(pair<double, double> data);
  bool Delete(double key);

 private:
  template <typename TYPE, typename ModelType>
  void CheckRoot(int c, int type, double time, double *optimalCost,
                 RootStruct *rootStruct);
  RootStruct ChooseRoot();
  template <typename TYPE, typename ModelType>
  TYPE ConstructRoot(const RootStruct &rootStruct, const IndexPair &range,
                     SubDataset *subDataset, int *childLeft);
  SubDataset StoreRoot(const RootStruct &rootStruct, NodeCost *nodeCost);

  void ConstructSubTree(const RootStruct &rootStruct,
                        const SubDataset &subDataset, NodeCost *nodeCost);

 private:
  NodeCost dp(const IndexPair &range);
  double CalculateEntropy(int size, int childNum,
                          const vector<DataRange> &perSize) const;
  template <typename TYPE>
  void NodePartition(const TYPE &node, const DataRange &range,
                     const vector<pair<double, double>> &dataset,
                     vector<DataRange> *subData) const;

  template <typename TYPE>
  void InnerDivideSingle(int c, const DataRange &range,
                         vector<DataRange> &subDataset);
  template <typename TYPE>
  TYPE InnerDivideAll(int c, const IndexPair &range, SubDataset *subDataset);
  template <typename TYPE>
  void CalInner(int c, int type, int frequency, double time,
                const IndexPair &dataRange, NodeCost *optimalCost,
                ParamStruct *optimalStruct);
  NodeCost dpInner(const IndexPair &dataRange);

  template <typename TYPE>
  double CalLeafFindTime(int actualSize, double density, const TYPE &node,
                         const DataRange &range) const;
  template <typename TYPE>
  double CalLeafInsertTime(int actualSize, double density, const TYPE &node,
                           const DataRange &range,
                           const DataRange &findRange) const;
  NodeCost dpLeaf(const IndexPair &dataRange);

  template <typename TYPE>
  void CheckGreedy(int c, int type, double pi, int frequency, double time,
                   const DataRange &range, ParamStruct *optimalStruct,
                   NodeCost *optimalCost);
  NodeCost GreedyAlgorithm(const IndexPair &range);

  template <typename TYPE>
  TYPE storeInnerNode(int storeIdx, const MapKey &key, const IndexPair &range);
  void storeOptimalNode(int storeIdx, int optimalType, const MapKey key,
                        const IndexPair &range);
  bool allocateEmptyBlock(int left, int len);
  int getIndex(int size);
  void initEntireData(int left, int size, bool reinit);
  int allocateMemory(int size);
  void releaseMemory(int left, int size);

  void initEntireChild(int size);
  int allocateChildMemory(int size);

  void initLR(LRModel *lr, const vector<pair<double, double>> &dataset);
  void initPLR(PLRModel *plr, const vector<pair<double, double>> &dataset);
  void initHis(HisModel *his, const vector<pair<double, double>> &dataset);
  void initBS(BSModel *bs, const vector<pair<double, double>> &dataset);
  void Train(LRModel *lr, const int left, const int size);
  void Train(PLRModel *plr, const int left, const int size);
  void Train(HisModel *his, const int left, const int size);
  void Train(BSModel *bs, const int left, const int size);

  void initArray(ArrayType *arr, const vector<pair<double, double>> &dataset,
                 int cap);  // for RMI
  void initArray(ArrayType *arr, const int left, const int size,
                 int cap);                                         // for expand
  void initArray(ArrayType *arr, const int left, const int size);  // for CARMI
  int UpdateError(ArrayType *arr);                                 // for CARMI
  int UpdateError(ArrayType *arr, const int start_idx,
                  const int size);  // for dp
  void Train(ArrayType *arr);
  void Train(ArrayType *arr, const int start_idx, const int size);

  void initGA(GappedArrayType *ga, const vector<pair<double, double>> &dataset,
              int cap);  // for RMI
  void initGA(GappedArrayType *ga, const int left, const int size,
              int cap);  // for expand
  void initGA(GappedArrayType *ga, const int left,
              const int size);           // for CARMI
  int UpdateError(GappedArrayType *ga);  // for CARMI
  int UpdateError(GappedArrayType *ga, const int start_idx,
                  const int size);  // for dp
  void Train(GappedArrayType *ga);
  void Train(GappedArrayType *ga, const int start_idx, const int size);

  void initYCSB(YCSBLeaf *ycsb, const int start_idx, const int size);
  int UpdateError(YCSBLeaf *ycsb, const int start_idx, const int size);
  void Train(YCSBLeaf *ycsb, const int start_idx, const int size);

  void UpdateLeaf();
  int ArrayBinarySearch(double key, int start, int end) const;
  int GABinarySearch(double key, int start_idx, int end_idx) const;
  int YCSBBinarySearch(double key, int start, int end) const;
  int TestBinarySearch(double key, int start, int end) const;

 public:
  CARMIRoot root;
  int rootType;
  double kRate;
  int kAlgorithmThreshold;
  const int kMaxKeyNum = 1024;
  const int kThreshold = 2;  // used to initialize a leaf node
  vector<BaseNode> entireChild;
  vector<pair<double, double>> entireData;

  int curr;  // for YCSB

 private:
  unsigned int entireChildNumber;
  unsigned int nowChildNumber;

  unsigned int nowDataSize;
  unsigned int entireDataSize;
  vector<EmptyBlock> emptyBlocks;

  map<double, int> scanLeaf;

  vector<pair<double, double>> initDataset;
  vector<pair<double, double>> findQuery;
  vector<pair<double, double>> insertQuery;

  map<DataRange, NodeCost> COST;
  map<MapKey, ParamStruct> structMap;
  int querySize;

  int kLeafNodeID;   // for static structure
  int kInnerNodeID;  // for static structure

 private:
  const NodeCost emptyCost = {0, 0, 0, false};
  const DataRange emptyRange = {-1, 0};
  const ParamStruct leafP = {5, 2, 0.5, vector<MapKey>()};

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
  friend class YCSBLeaf;
};

// RMI
CARMI::CARMI(const vector<pair<double, double>> &dataset, int childNum,
             int kInnerID, int kLeafID) {
  nowDataSize = 0;
  kLeafNodeID = kLeafID;
  kInnerNodeID = kInnerID;
  initEntireData(0, dataset.size(), false);
  initEntireChild(dataset.size());
  int left = allocateChildMemory(childNum);

  vector<vector<pair<double, double>>> perSubDataset;
  vector<pair<double, double>> tmp;
  for (int i = 0; i < childNum; i++) perSubDataset.push_back(tmp);
  rootType = kInnerID;

  switch (kInnerNodeID) {
    case 0: {
      root.lrRoot = LRType(childNum);
      root.lrRoot.childLeft = left;

      root.lrRoot.model.Train(dataset, childNum);

      for (int i = 0; i < dataset.size(); i++) {
        int p = root.lrRoot.model.Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
      }

      for (int i = 0; i < childNum; i++) {
        LRModel innerLR;
        innerLR.SetChildNumber(32);
        initLR(&innerLR, perSubDataset[i]);
        entireChild[left + i].lr = innerLR;
      }
    } break;
    case 1: {
      root.plrRoot = PLRType(childNum);
      root.plrRoot.childLeft = left;

      root.plrRoot.model.Train(dataset, childNum);

      for (int i = 0; i < dataset.size(); i++) {
        int p = root.plrRoot.model.Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
      }

      for (int i = 0; i < childNum; i++) {
        PLRModel innerPLR;
        innerPLR.SetChildNumber(32);
        initPLR(&innerPLR, perSubDataset[i]);
        entireChild[left + i].plr = innerPLR;
      }
    } break;
    case 3: {
      root.bsRoot = BSType(childNum);
      root.bsRoot.childLeft = left;

      root.bsRoot.model.Train(dataset, childNum);

      for (int i = 0; i < dataset.size(); i++) {
        int p = root.bsRoot.model.Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
      }

      for (int i = 0; i < childNum; i++) {
        BSModel innerBS;
        innerBS.SetChildNumber(32);
        initBS(&innerBS, perSubDataset[i]);
        entireChild[left + i].bs = innerBS;
      }
    } break;
    case 2: {
      root.hisRoot = HisType(childNum);
      root.hisRoot.childLeft = left;

      root.hisRoot.model.Train(dataset, childNum);

      for (int i = 0; i < dataset.size(); i++) {
        int p = root.hisRoot.model.Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
      }

      for (int i = 0; i < childNum; i++) {
        HisModel innerHis;
        innerHis.SetChildNumber(32);
        initHis(&innerHis, perSubDataset[i]);
        entireChild[left + i].his = innerHis;
      }
    } break;
  }
};

#endif  // !CARMI_H
