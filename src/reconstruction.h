#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "params.h"
#include "innerNodeType/bin_type.h"
#include "innerNodeType/his_type.h"
#include "innerNodeType/lr_type.h"
#include "innerNodeType/nn_type.h"
#include "leafNodeType/ga_type.h"
#include "leafNodeType/array_type.h"
#include "inlineFunction.h"
#include "function.h"
#include <float.h>
#include <vector>
using namespace std;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

vector<vector<pair<double, double>>> tmpEntireDataset;

vector<LRType> tmpLRVec;
vector<NNType> tmpNNVec;
vector<HisType> tmpHisVec;
vector<BSType> tmpBSVec;

extern double maxSpace;
extern int kMaxKeyNum;
extern double kRate;
extern const double kReadWriteRate;

pair<int, int> ChooseRoot(const vector<pair<double, double>> &dataset, const double maxSpace)
{
    double OptimalValue = DBL_MAX;
    int space;
    int c;
    int optimalChildNumber, optimalType;
    for (int c = 1024; c <= dataset.size();)
    {
        if (c <= 4096)
            c *= 2;
        else if (c <= 40960)
            c += 8192;
        else if (c <= 1000000)
            c += 65536;
        else
            c *= 2;
        for (int type = 0; type < 4; type++)
        {
            double time;
            switch (type)
            {
            case 0:
            {
                space = (4 * c + sizeof(LRType) + sizeof(ArrayType) * c);
                if (space > maxSpace)
                    continue;
                LRVector.push_back(LRType(c));
                LRVector[0].Initialize(dataset);
                time = 8.1624 * 1e-9 * dataset.size();
                break;
            }
            case 1:
            {
                space = (4 * c + 192 + sizeof(NNType) + sizeof(ArrayType) * c);
                if (space > maxSpace)
                    continue;
                NNVector.push_back(NNType(c));
                NNVector[0].Initialize(dataset);
                time = 20.26894 * 1e-9 * dataset.size();
                break;
            }
            case 2:
            {
                space = (5 * c + sizeof(HisType) + sizeof(ArrayType) * c);
                if (space > maxSpace)
                    continue;
                HisVector.push_back(HisType(c));
                HisVector[0].Initialize(dataset);
                time = 19.6543 * 1e-9 * dataset.size();
                break;
            }
            case 3:
            {
                space = (12 * c + sizeof(BSType) + sizeof(ArrayType) * c);
                if (space > maxSpace)
                    continue;
                BSVector.push_back(BSType(c));
                BSVector[0].Initialize(dataset);
                time = 4 * log(c) / log(2) * 1e-9 * dataset.size();
                break;
            }
            }

            auto entropy = GetEntropy(dataset.size());
            // entropy /= (log(c) / log(2));
            double ratio = time / entropy;
            if (ratio <= OptimalValue)
            {
                optimalChildNumber = c;
                optimalType = type;
                OptimalValue = ratio;
            }
            vector<LRType>().swap(LRVector);
            vector<NNType>().swap(NNVector);
            vector<HisType>().swap(HisVector);
            vector<BSType>().swap(BSVector);
            vector<ArrayType>().swap(ArrayVector);
            vector<GappedArrayType>().swap(GAVector);
        }
    }
    vector<vector<pair<double, double>>>().swap(entireDataset);
    vector<vector<pair<double, double>>>().swap(tmpEntireDataset);
    entireDataset.clear();
    tmpEntireDataset.clear();
    cout << "Best type is: " << optimalType << "\tbest childNumber: " << optimalChildNumber << "\tOptimal Value: " << OptimalValue << endl;
    return {optimalType, optimalChildNumber};
}

// store the optimal node into the index structure
// tmpIdx: idx in tmpVector
int storeOptimalNode(int optimalType, int tmpIdx, const vector<pair<double, double>> &findData)
{
    int idx;
    switch (optimalType)
    {
    case 0:
    {
        LRVector.push_back(tmpLRVec[tmpIdx]);
        idx = LRVector.size() - 1;
        int optimalChildNumber = LRVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
            subFindData.push_back(tmp);
        for (int i = 0; i < findData.size(); i++)
        {
            int p = LRVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = LRVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i]);
            LRVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        break;
    }
    case 1:
    {
        NNVector.push_back(tmpNNVec[tmpIdx]);
        idx = NNVector.size() - 1;
        int optimalChildNumber = NNVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
            subFindData.push_back(tmp);
        for (int i = 0; i < findData.size(); i++)
        {
            int p = NNVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = NNVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i]);
            NNVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        break;
    }
    case 2:
    {
        HisVector.push_back(tmpHisVec[tmpIdx]);
        idx = HisVector.size() - 1;
        int optimalChildNumber = HisVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
            subFindData.push_back(tmp);
        for (int i = 0; i < findData.size(); i++)
        {
            int p = HisVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = HisVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i]);
            HisVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        break;
    }
    case 3:
    {
        BSVector.push_back(tmpBSVec[tmpIdx]);
        idx = BSVector.size() - 1;
        int optimalChildNumber = BSVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
            subFindData.push_back(tmp);
        for (int i = 0; i < findData.size(); i++)
        {
            int p = BSVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }
        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = BSVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i]);
            BSVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        break;
    }
    case 4:
    {
        // choose an array node as the leaf node
        ArrayVector.push_back(ArrayType(kMaxKeyNum));
        idx = ArrayVector.size() - 1;
        ArrayVector[idx].SetDataset(findData);
        break;
    }
    case 5:
    {
        GAVector.push_back(GappedArrayType(kMaxKeyNum));
        idx = GAVector.size() - 1;
        GAVector[idx].SetDataset(findData);
        break;
    }
    }
    return idx;
}

pair<pair<double, double>, int> Construct(bool isLeaf, const vector<pair<double, double>> &findData, const vector<int> &readCnt, const vector<pair<double, double>> &insertData, const vector<int> &writeCnt, double maxSpace)
{
    // construct a leaf node
    if (isLeaf || maxSpace <= 0)
    {
        int L; // stored in the child vector of the upper node
        double space;
        int idx, type;
        chrono::_V2::system_clock::time_point s, e;
        double time;
        int read = 0, write = 0;
        for (int i = 0; i < readCnt.size(); i++)
            read += readCnt[i];
        for (int i = 0; i < writeCnt.size(); i++)
            write += writeCnt[i];
        if (write == 0 || float(read) / (float(read + write)) >= kReadWriteRate)
        {
            // choose an array node as the leaf node
            auto tmp = ArrayType(kMaxKeyNum);
            TestArraySetDataset(tmp, findData);
            L = 0x40000000;
            space = sizeof(ArrayType);
            type = 4;
            s = chrono::system_clock::now();
            for (int i = 0; i < findData.size(); i++)
                TestArrayFind(tmp, findData[i].first);
            for (int i = 0; i < insertData.size(); i++)
                TestArrayInsert(tmp, insertData[i]);
            e = chrono::system_clock::now();
            time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        }
        else
        {
            // choose a gapped array node as the leaf node
            auto tmp = GappedArrayType(kMaxKeyNum);
            TestGappedArraySetDataset(tmp, findData);
            L = 0x50000000;
            space = sizeof(GappedArrayType);
            type = 5;
            s = chrono::system_clock::now();
            for (int i = 0; i < findData.size(); i++)
                TestGappedArrayFind(tmp, findData[i].first);
            for (int i = 0; i < insertData.size(); i++)
                TestGappedArrayInsert(tmp, insertData[i]);
            e = chrono::system_clock::now();
            time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        }
        tmpEntireDataset.pop_back();
        double cost = time + space * kRate / 1024 / 1024;
        return {{cost, space}, L};
    }
    else
    {
        double OptimalValue = DBL_MAX;
        int space;
        int c;
        int optimalChildNumber = 32, optimalType = 0, optimalSpace = 0;
        int tmpIdx;
        for (int k = 16; k < findData.size();)
        {
            c = k;
            if (k < 4096)
                k *= 2;
            else if (k < 40960)
                k += 8192;
            else if (k <= 1000000)
                k += 65536;
            else
                k *= 2;
            if (512 * c < findData.size())
                continue;
            for (int type = 0; type < 4; type++)
            {
                switch (type)
                {
                case 0:
                {
                    space = (4 * c + sizeof(LRType));
                    if (space > maxSpace)
                        continue;
                    tmpLRVec.push_back(LRType(c));
                    int idx = tmpLRVec.size() - 1;
                    tmpLRVec[idx].model.Train(findData, c);

                    // divide the key and query
                    vector<vector<pair<double, double>>> subFindData;
                    vector<vector<pair<double, double>>> subInsertData;
                    vector<vector<int>> subReadCnt;
                    vector<vector<int>> subWriteCnt;
                    vector<pair<double, double>> tmp;
                    vector<int> tmp1;
                    for (int i = 0; i < c; i++)
                    {
                        subFindData.push_back(tmp);
                        subInsertData.push_back(tmp);
                        subReadCnt.push_back(tmp1);
                        subWriteCnt.push_back(tmp1);
                    }
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = tmpLRVec[idx].model.Predict(findData[i].first);
                        subFindData[p].push_back(findData[i]);
                        subReadCnt[p].push_back(readCnt[i]);
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = tmpLRVec[idx].model.Predict(insertData[i].first);
                        subInsertData[p].push_back(insertData[i]);
                        subWriteCnt[p].push_back(writeCnt[i]);
                    }

                    // only record the time of inner node using cost model
                    double time = 8.1624 * 1e-9 * (findData.size() + insertData.size());
                    double RootCost = time + kRate * space / 1024 / 1024;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space);  // construct a leaf node
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space); // construct an inner node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space);
                        tmpLRVec[idx].child.push_back(res.second);
                        RootCost += res.first.first;
                        space += res.first.second;
                        if (space > maxSpace)
                        {
                            RootCost = DBL_MAX;
                            break;
                        }
                    }
                    if (RootCost <= OptimalValue)
                    {
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                        optimalSpace = space;
                    }

                    vector<vector<pair<double, double>>>().swap(subFindData);
                    vector<vector<pair<double, double>>>().swap(subInsertData);
                    vector<vector<int>>().swap(subReadCnt);
                    vector<vector<int>>().swap(subWriteCnt);
                    break;
                }
                case 1:
                {
                    space = (4 * c + 192 + sizeof(NNType));
                    if (space > maxSpace)
                        continue;
                    tmpNNVec.push_back(NNType(c));
                    int idx = tmpNNVec.size() - 1;
                    tmpNNVec[idx].model.Train(findData, c);

                    // divide the key and query
                    vector<vector<pair<double, double>>> subFindData;
                    vector<vector<pair<double, double>>> subInsertData;
                    vector<vector<int>> subReadCnt;
                    vector<vector<int>> subWriteCnt;
                    vector<pair<double, double>> tmp;
                    vector<int> tmp1;
                    for (int i = 0; i < c; i++)
                    {
                        subFindData.push_back(tmp);
                        subInsertData.push_back(tmp);
                        subReadCnt.push_back(tmp1);
                        subWriteCnt.push_back(tmp1);
                    }
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = tmpNNVec[idx].model.Predict(findData[i].first);
                        subFindData[p].push_back(findData[i]);
                        subReadCnt[p].push_back(readCnt[i]);
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = tmpNNVec[idx].model.Predict(insertData[i].first);
                        subInsertData[p].push_back(insertData[i]);
                        subWriteCnt[p].push_back(writeCnt[i]);
                    }

                    // only record the time of inner node using cost model
                    double time = 20.2689 * 1e-9 * (findData.size() + insertData.size());

                    double RootCost = time + kRate * space / 1024 / 1024;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space); // construct an inner node
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space);  // construct a leaf node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space);
                        tmpNNVec[idx].child.push_back(res.second);
                        RootCost += res.first.first;
                        space += res.first.second;
                        if (space > maxSpace)
                        {
                            RootCost = DBL_MAX;
                            break;
                        }
                    }
                    if (RootCost <= OptimalValue)
                    {
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                        optimalSpace = space;
                    }

                    vector<vector<pair<double, double>>>().swap(subFindData);
                    vector<vector<pair<double, double>>>().swap(subInsertData);
                    vector<vector<int>>().swap(subReadCnt);
                    vector<vector<int>>().swap(subWriteCnt);
                    break;
                }
                case 2:
                {
                    space = (5 * c + sizeof(HisType));
                    if (space > maxSpace)
                        continue;
                    tmpHisVec.push_back(HisType(c));
                    int idx = tmpHisVec.size() - 1;
                    tmpHisVec[idx].model.Train(findData, c);

                    // divide the key and query
                    vector<vector<pair<double, double>>> subFindData;
                    vector<vector<pair<double, double>>> subInsertData;
                    vector<vector<int>> subReadCnt;
                    vector<vector<int>> subWriteCnt;
                    vector<pair<double, double>> tmp;
                    vector<int> tmp1;
                    for (int i = 0; i < c; i++)
                    {
                        subFindData.push_back(tmp);
                        subInsertData.push_back(tmp);
                        subReadCnt.push_back(tmp1);
                        subWriteCnt.push_back(tmp1);
                    }
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = tmpHisVec[idx].model.Predict(findData[i].first);
                        subFindData[p].push_back(findData[i]);
                        subReadCnt[p].push_back(readCnt[i]);
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = tmpHisVec[idx].model.Predict(insertData[i].first);
                        subInsertData[p].push_back(insertData[i]);
                        subWriteCnt[p].push_back(writeCnt[i]);
                    }

                    // only record the time of inner node using cost model
                    double time = 19.6543 * 1e-9 * (findData.size() + insertData.size());

                    double RootCost = time + kRate * space / 1024 / 1024;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space); // construct an inner node
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space);  // construct a leaf node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space);
                        tmpHisVec[idx].child.push_back(res.second);
                        RootCost += res.first.first;
                        space += res.first.second;
                        if (space > maxSpace)
                        {
                            RootCost = DBL_MAX;
                            break;
                        }
                    }
                    if (RootCost <= OptimalValue)
                    {
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                        optimalSpace = space;
                    }

                    vector<vector<pair<double, double>>>().swap(subFindData);
                    vector<vector<pair<double, double>>>().swap(subInsertData);
                    vector<vector<int>>().swap(subReadCnt);
                    vector<vector<int>>().swap(subWriteCnt);
                    break;
                }
                case 3:
                {
                    space = (12 * c + sizeof(BSType));
                    if (space > maxSpace)
                        continue;
                    tmpBSVec.push_back(BSType(c));
                    int idx = tmpBSVec.size() - 1;
                    tmpBSVec[idx].model.Train(findData, c);

                    // divide the key and query
                    vector<vector<pair<double, double>>> subFindData;
                    vector<vector<pair<double, double>>> subInsertData;
                    vector<vector<int>> subReadCnt;
                    vector<vector<int>> subWriteCnt;
                    vector<pair<double, double>> tmp;
                    vector<int> tmp1;
                    for (int i = 0; i < c; i++)
                    {
                        subFindData.push_back(tmp);
                        subInsertData.push_back(tmp);
                        subReadCnt.push_back(tmp1);
                        subWriteCnt.push_back(tmp1);
                    }
                    for (int i = 0; i < findData.size(); i++)
                    {
                        int p = tmpBSVec[idx].model.Predict(findData[i].first);
                        subFindData[p].push_back(findData[i]);
                        subReadCnt[p].push_back(readCnt[i]);
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = tmpBSVec[idx].model.Predict(insertData[i].first);
                        subInsertData[p].push_back(insertData[i]);
                        subWriteCnt[p].push_back(writeCnt[i]);
                    }

                    // only record the time of inner node using cost model
                    double time = 4 * log(c) / log(2) * 1e-9 * (findData.size() + insertData.size());

                    double RootCost = time + kRate * space / 1024 / 1024;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space); // construct an inner node
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space);  // construct a leaf node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace - space);
                        tmpBSVec[idx].child.push_back(res.second);
                        RootCost += res.first.first;
                        space += res.first.second;
                        if (space > maxSpace)
                        {
                            RootCost = DBL_MAX;
                            break;
                        }
                    }
                    if (RootCost <= OptimalValue)
                    {
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                        optimalSpace = space;
                    }

                    vector<vector<pair<double, double>>>().swap(subFindData);
                    vector<vector<pair<double, double>>>().swap(subInsertData);
                    vector<vector<int>>().swap(subReadCnt);
                    vector<vector<int>>().swap(subWriteCnt);
                    break;
                }
                }
            }
        }
        return {{OptimalValue, optimalSpace}, (optimalType << 28) + tmpIdx};
    }
}

// main function of construction
// return the type of root
// findDataset: the dataset used to initialize the index
// insertDataset: the dataset to be inserted into the index
// readCnt: the number of READ corresponding to each key
// writeCnt: the number of WRITE corresponding to each key
int Construction(double maxSpace, const vector<pair<double, double>> &findDataset, const vector<int> &readCnt, const vector<pair<double, double>> &insertDataset, const vector<int> &writeCnt)
{
    cout << endl;
    cout << "-------------------------------" << endl;
    cout << "Start construction!" << endl;

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
    vector<vector<pair<double, double>>>().swap(entireDataset);
    vector<vector<pair<double, double>>>().swap(tmpEntireDataset);
    entireDataset.clear();
    tmpEntireDataset.clear();

    auto res = ChooseRoot(findDataset, maxSpace);
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
        maxSpace -= (4 * childNum + sizeof(LRType) + sizeof(ArrayType) * childNum);
        break;
    }
    case 1:
    {
        NNVector.push_back(NNType(childNum));
        NNVector[0].model.Train(findDataset, childNum);
        maxSpace -= (4 * childNum + 192 + sizeof(NNType) + sizeof(ArrayType) * childNum);
        break;
    }
    case 2:
    {
        HisVector.push_back(HisType(childNum));
        HisVector[0].model.Train(findDataset, childNum);
        maxSpace -= (5 * childNum + sizeof(HisType) + sizeof(ArrayType) * childNum);
        break;
    }
    case 3:
    {
        BSVector.push_back(BSType(childNum));
        BSVector[0].model.Train(findDataset, childNum);
        maxSpace -= (12 * childNum + sizeof(BSType) + sizeof(ArrayType) * childNum);
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
            if ((subFindData[i].size() + subInsertData[i].size()) >= kMaxKeyNum)
            {
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);  // construct a leaf node
                if (res0.first.first > res1.first.first)
                    resChild = res1;
                else
                    resChild = res0;
                int childType = resChild.second >> 28;
                idx = storeOptimalNode(resChild.second >> 28, (resChild.second & 0x0FFFFFFF), subFindData[i]);
                idx += (childType << 28);
            }
            else
            {
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                idx = storeOptimalNode(resChild.second >> 28, 0, subFindData[i]);
                idx += resChild.second;
            }
            LRVector[0].child.push_back(idx);
            if ((resChild.second >> 28) > 4)
                maxSpace -= 8;
            else if ((resChild.second >> 28) < 4)
            {
                maxSpace += sizeof(ArrayType);
                maxSpace -= resChild.first.second;
            }
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
            if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
            {
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);  // construct a leaf node
                if (res0.first.first > res1.first.first)
                    resChild = res1;
                else
                    resChild = res0;
                int childType = resChild.second >> 28;
                idx = storeOptimalNode(resChild.second >> 28, (resChild.second & 0x0FFFFFFF), subFindData[i]);
                idx += (childType << 28);
            }
            else
            {
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                idx = storeOptimalNode(resChild.second >> 28, 0, subFindData[i]);
                idx += resChild.second;
            }
            if ((resChild.second >> 28) > 4)
                maxSpace -= 8;
            else if ((resChild.second >> 28) < 4)
            {
                maxSpace += sizeof(ArrayType);
                maxSpace -= resChild.first.second;
            }
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
            if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
            {
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);  // construct a leaf node
                if (res0.first.first > res1.first.first)
                    resChild = res1;
                else
                    resChild = res0;
                int childType = resChild.second >> 28;
                idx = storeOptimalNode(resChild.second >> 28, (resChild.second & 0x0FFFFFFF), subFindData[i]);
                idx += (childType << 28);
            }
            else
            {
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                idx = storeOptimalNode(resChild.second >> 28, 0, subFindData[i]);
                idx += resChild.second;
            }
            if ((resChild.second >> 28) > 4)
                maxSpace -= 8;
            else if ((resChild.second >> 28) < 4)
            {
                maxSpace += sizeof(ArrayType);
                maxSpace -= resChild.first.second;
            }
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
            if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
            {
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);  // construct a leaf node
                if (res0.first.first > res1.first.first)
                    resChild = res1;
                else
                    resChild = res0;
                int childType = resChild.second >> 28;
                idx = storeOptimalNode(resChild.second >> 28, (resChild.second & 0x0FFFFFFF), subFindData[i]);
                idx += (childType << 28);
            }
            else
            {
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                idx = storeOptimalNode(resChild.second >> 28, 0, subFindData[i]);
                idx += resChild.second;
            }
            if ((resChild.second >> 28) > 4)
                maxSpace -= 8;
            else if ((resChild.second >> 28) < 4)
            {
                maxSpace += sizeof(ArrayType);
                maxSpace -= resChild.first.second;
            }
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