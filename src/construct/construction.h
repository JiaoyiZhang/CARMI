#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H

#include "greedy.h"
#include "store_node.h"
#include "dp.h"
using namespace std;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

extern pair<double, double> *entireData;

vector<LRType> tmpLRVec;
vector<NNType> tmpNNVec;
vector<HisType> tmpHisVec;
vector<BSType> tmpBSVec;
vector<ArrayType> tmpArrayVec;
vector<GappedArrayType> tmpGAVec;

extern int kMaxKeyNum;
extern double kRate;

// main function of construction
// return the type of root
// findDataset: the dataset used to initialize the index
// insertDataset: the dataset to be inserted into the index
// readCnt: the number of READ corresponding to each key
// writeCnt: the number of WRITE corresponding to each key
int Construction(const vector<pair<double, double>> &findDataset, const vector<int> &readCnt, const vector<pair<double, double>> &insertDataset, const vector<int> &writeCnt)
{
    cout << endl;
    cout << "-------------------------------" << endl;
    cout << "Start construction!" << endl;
    initEntireData(findDataset.size() + insertDataset.size());

    auto res = ChooseRoot(findDataset);

    vector<LRType>().swap(LRVector);
    vector<NNType>().swap(NNVector);
    vector<HisType>().swap(HisVector);
    vector<BSType>().swap(BSVector);
    vector<ArrayType>().swap(ArrayVector);
    vector<GappedArrayType>().swap(GAVector);
    vector<LRType>().swap(tmpLRVec);
    vector<NNType>().swap(tmpNNVec);
    vector<HisType>().swap(tmpHisVec);
    vector<BSType>().swap(tmpBSVec);
    int childNum = res.second;
    int rootType = res.first;
    cout << "Construction of the root node has been completed!" << endl;
    cout << "The optimal value of root is: " << res.first << ",\tthe optimal child number is: " << res.second << endl;
    switch (rootType)
    {
    case 0:
    {
        LRVector.push_back(LRType(childNum));
        LRVector[0].model.Train(findDataset, childNum);
        break;
    }
    case 1:
    {
        NNVector.push_back(NNType(childNum));
        NNVector[0].model.Train(findDataset, childNum);
        break;
    }
    case 2:
    {
        HisVector.push_back(HisType(childNum));
        HisVector[0].model.Train(findDataset, childNum);
        break;
    }
    case 3:
    {
        BSVector.push_back(BSType(childNum));
        BSVector[0].model.Train(findDataset, childNum);
        break;
    }
    }
    double totalCost = 0.0;
    vector<vector<pair<double, double>>> subFindData;
    vector<vector<pair<double, double>>> subInsertData;
    vector<vector<int>> subReadCnt;
    vector<vector<int>> subWriteCnt;
    vector<pair<double, double>> tmp;
    vector<int> tmp1;
    for (int i = 0; i < childNum; i++)
    {
        subFindData.push_back(tmp);
        subInsertData.push_back(tmp);
        subReadCnt.push_back(tmp1);
        subWriteCnt.push_back(tmp1);
    }
    switch (rootType)
    {
    case 0:
    {
        for (int i = 0; i < findDataset.size(); i++)
        {
            int p = LRVector[0].model.Predict(findDataset[i].first);
            subFindData[p].push_back(findDataset[i]);
            subReadCnt[p].push_back(readCnt[i]);
        }
        for (int i = 0; i < insertDataset.size(); i++)
        {
            int p = LRVector[0].model.Predict(insertDataset[i].first);
            subInsertData[p].push_back(insertDataset[i]);
            subWriteCnt[p].push_back(writeCnt[i]);
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, int> resChild;
            int idx;
            if ((subFindData[i].size() + subInsertData[i].size()) > 4096)
                resChild = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
            else if ((subFindData[i].size() + subInsertData[i].size()) >= kMaxKeyNum)
            {
                cout << "construct child i:" << i << endl;
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);  // construct a leaf node
                if (res0.first.first > res1.first.first)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);
            }
            int childType = resChild.second >> 28;
            idx = storeOptimalNode(resChild.second >> 28, (resChild.second & 0x0FFFFFFF), subFindData[i]);
            idx += (childType << 28);
            LRVector[0].child.push_back(idx);
            totalCost += resChild.first.first;
        }
        break;
    }
    case 1:
    {
        for (int i = 0; i < findDataset.size(); i++)
        {
            int p = NNVector[0].model.Predict(findDataset[i].first);
            subFindData[p].push_back(findDataset[i]);
            subReadCnt[p].push_back(readCnt[i]);
        }
        for (int i = 0; i < insertDataset.size(); i++)
        {
            int p = NNVector[0].model.Predict(insertDataset[i].first);
            subInsertData[p].push_back(insertDataset[i]);
            subWriteCnt[p].push_back(writeCnt[i]);
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, int> resChild;
            int idx;
            if ((subFindData[i].size() + subInsertData[i].size()) > 4096)
                resChild = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
            else if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
            {
                cout << "construct child i:" << i << endl;
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);  // construct a leaf node
                if (res0.first.first > res1.first.first)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);
            }
            int childType = resChild.second >> 28;
            idx = storeOptimalNode(resChild.second >> 28, (resChild.second & 0x0FFFFFFF), subFindData[i]);
            idx += (childType << 28);
            NNVector[0].child.push_back(idx);
            totalCost += resChild.first.first;
        }
    }
    break;
    case 2:
    {
        for (int i = 0; i < findDataset.size(); i++)
        {
            int p = HisVector[0].model.Predict(findDataset[i].first);
            subFindData[p].push_back(findDataset[i]);
            subReadCnt[p].push_back(readCnt[i]);
        }
        for (int i = 0; i < insertDataset.size(); i++)
        {
            int p = HisVector[0].model.Predict(insertDataset[i].first);
            subInsertData[p].push_back(insertDataset[i]);
            subWriteCnt[p].push_back(writeCnt[i]);
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, int> resChild;
            int idx;
            if ((subFindData[i].size() + subInsertData[i].size()) > 4096)
                resChild = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
            else if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
            {
                cout << "construct child i:" << i << endl;
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);  // construct a leaf node
                if (res0.first.first > res1.first.first)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);
            }
            int childType = resChild.second >> 28;
            idx = storeOptimalNode(resChild.second >> 28, (resChild.second & 0x0FFFFFFF), subFindData[i]);
            idx += (childType << 28);
            HisVector[0].child.push_back(idx);
            totalCost += resChild.first.first;
        }
    }
    break;
    case 3:
    {
        for (int i = 0; i < findDataset.size(); i++)
        {
            int p = BSVector[0].model.Predict(findDataset[i].first);
            subFindData[p].push_back(findDataset[i]);
            subReadCnt[p].push_back(readCnt[i]);
        }
        for (int i = 0; i < insertDataset.size(); i++)
        {
            int p = BSVector[0].model.Predict(insertDataset[i].first);
            subInsertData[p].push_back(insertDataset[i]);
            subWriteCnt[p].push_back(writeCnt[i]);
        }

        for (int i = 0; i < childNum; i++)
        {
            pair<pair<double, double>, int> resChild;
            int idx;
            if ((subFindData[i].size() + subInsertData[i].size()) > 4096)
                resChild = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
            else if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
            {
                cout << "construct child i:" << i << endl;
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);  // construct a leaf node
                if (res0.first.first > res1.first.first)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
            {
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);
            }
            int childType = resChild.second >> 28;
            idx = storeOptimalNode(resChild.second >> 28, (resChild.second & 0x0FFFFFFF), subFindData[i]);
            idx += (childType << 28);
            BSVector[0].child.push_back(idx);
            totalCost += resChild.first.first;
        }
    }
    break;
    }
    cout << "total cost: " << totalCost << endl;
    return rootType;
}

#endif