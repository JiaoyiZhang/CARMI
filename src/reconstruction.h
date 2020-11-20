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

vector<LRType> tmpLRVec;
vector<NNType> tmpNNVec;
vector<HisType> tmpHisVec;
vector<BSType> tmpBSVec;

extern int kMaxSpace;
extern int kMaxKeyNum;
extern double kRate;
extern const double kReadWriteRate;

pair<int, int> ChooseRoot(const vector<pair<double, double>> &dataset, const int maxSpace)
{
    double OptimalValue = DBL_MAX;

    vector<int> candidateChildNum = {2000, 3000, 3907, 5000, 7500, 10000, 12500, 15000, 17500, 20000, 25000, 50000, 60000, 70125, 80000, 90000, 100000, 200000, 350000};

    int space;
    int c;
    int optimalChildNumber, optimalType;
    cout << "Choose root!" << endl;
    for (int i = 0; i < candidateChildNum.size(); i++)
    {
        c = candidateChildNum[i];
        if (c * 512 < dataset.size() || c * 10 > dataset.size())
            continue;
        // cout << "Now child number: " << c << endl;
        for (int type = 0; type < 4; type++)
        {
            switch (type)
            {
            case 0:
            {
                space = 4 * c + 24;
                if (space > maxSpace)
                    continue;
                LRVector.push_back(LRType(c));
                LRVector[0].Initialize(dataset);
                break;
            }
            case 1:
            {
                space = 4 * c + 200;
                if (space > maxSpace)
                    continue;
                NNVector.push_back(NNType(c));
                NNVector[0].Initialize(dataset);
                break;
            }
            case 2:
            {
                space = c + 16;
                if (space > maxSpace)
                    continue;
                HisVector.push_back(HisType(c));
                HisVector[0].Initialize(dataset);
                break;
            }
            case 3:
            {
                space = 12 * c + 8;
                if (space > maxSpace)
                    continue;
                BSVector.push_back(BSType(c));
                BSVector[0].Initialize(dataset);
                break;
            }
            }
            // only record the time of inner node
            chrono::_V2::system_clock::time_point s, e;
            s = chrono::system_clock::now();
            for (int i = 0; i < dataset.size(); i++)
                InnerNodeTime(0, type, dataset[i].first);
            e = chrono::system_clock::now();
            double time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

            auto entropy = GetEntropy(dataset.size());
            entropy /= (log(c) / log(2));
            double ratio = time / entropy;
            if (ratio < OptimalValue)
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
    cout << "Best type is: " << optimalType << "\tbest childNumber: " << optimalChildNumber << "\tOptimal Value: " << OptimalValue << endl;
    return {optimalType, optimalChildNumber};
}

// store the optimal node into the index structure
// tmpIdx: idx in tmpVector
int storeOptimalNode(int optimalType, int tmpIdx, const vector<pair<double, double>> &findData, const vector<pair<double, double>> &insertData)
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
        vector<vector<pair<double, double>>> subInsertData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
        {
            subFindData.push_back(tmp);
            subInsertData.push_back(tmp);
        }
        for (int i = 0; i < findData.size(); i++)
        {
            int p = LRVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }
        for (int i = 0; i < insertData.size(); i++)
        {
            int p = LRVector[idx].model.Predict(insertData[i].first);
            subInsertData[p].push_back(insertData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = LRVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i], subInsertData[i]);
            LRVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        vector<vector<pair<double, double>>>().swap(subInsertData);
        break;
    }
    case 1:
    {
        NNVector.push_back(tmpNNVec[tmpIdx]);
        idx = NNVector.size() - 1;
        int optimalChildNumber = NNVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<vector<pair<double, double>>> subInsertData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
        {
            subFindData.push_back(tmp);
            subInsertData.push_back(tmp);
        }
        for (int i = 0; i < findData.size(); i++)
        {
            int p = NNVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }
        for (int i = 0; i < insertData.size(); i++)
        {
            int p = NNVector[idx].model.Predict(insertData[i].first);
            subInsertData[p].push_back(insertData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = NNVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i], subInsertData[i]);
            NNVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        vector<vector<pair<double, double>>>().swap(subInsertData);
        break;
    }
    case 2:
    {
        HisVector.push_back(tmpHisVec[tmpIdx]);
        idx = HisVector.size() - 1;
        int optimalChildNumber = HisVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<vector<pair<double, double>>> subInsertData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
        {
            subFindData.push_back(tmp);
            subInsertData.push_back(tmp);
        }
        for (int i = 0; i < findData.size(); i++)
        {
            int p = HisVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }
        for (int i = 0; i < insertData.size(); i++)
        {
            int p = HisVector[idx].model.Predict(insertData[i].first);
            subInsertData[p].push_back(insertData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = HisVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i], subInsertData[i]);
            HisVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        vector<vector<pair<double, double>>>().swap(subInsertData);
        break;
    }
    case 3:
    {
        BSVector.push_back(tmpBSVec[tmpIdx]);
        idx = BSVector.size() - 1;
        int optimalChildNumber = BSVector[idx].childNumber;
        // divide the key and query
        vector<vector<pair<double, double>>> subFindData;
        vector<vector<pair<double, double>>> subInsertData;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < optimalChildNumber; i++)
        {
            subFindData.push_back(tmp);
            subInsertData.push_back(tmp);
        }
        for (int i = 0; i < findData.size(); i++)
        {
            int p = BSVector[idx].model.Predict(findData[i].first);
            subFindData[p].push_back(findData[i]);
        }
        for (int i = 0; i < insertData.size(); i++)
        {
            int p = BSVector[idx].model.Predict(insertData[i].first);
            subInsertData[p].push_back(insertData[i]);
        }

        for (int i = 0; i < optimalChildNumber; i++)
        {
            int content = BSVector[idx].child[i];
            int type = content >> 28;
            int newIdx = content & 0x0FFFFFFF;
            auto actualIdx = storeOptimalNode(type, newIdx, subFindData[i], subInsertData[i]);
            BSVector[idx].child[i] = ((type << 28) + actualIdx);
        }

        vector<vector<pair<double, double>>>().swap(subFindData);
        vector<vector<pair<double, double>>>().swap(subInsertData);
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

pair<double, int> Construct(bool isLeaf, const vector<pair<double, double>> &findData, const vector<int> &readCnt, const vector<pair<double, double>> &insertData, const vector<int> &writeCnt, int maxSpace)
{
    // construct a leaf node
    if (isLeaf)
    // if (findData.size() + insertData.size() < kMaxKeyNum)
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
        if ((read == 0 && write == 0) || float(read) / (float(read + write)) >= kReadWriteRate)
        {
            // choose an array node as the leaf node
            auto tmp = ArrayType(kMaxKeyNum);
            tmp.SetDataset(findData);
            L = 0x40000000;
            space = 36 + findData.size() * 4;
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
            tmp.SetDataset(findData);
            L = 0x50000000;
            space = 44 + findData.size() * 4 / kDensity;
            type = 5;
            s = chrono::system_clock::now();
            for (int i = 0; i < findData.size(); i++)
                TestGappedArrayFind(tmp, findData[i].first);
            for (int i = 0; i < insertData.size(); i++)
                TestGappedArrayInsert(tmp, insertData[i]);
            e = chrono::system_clock::now();
            time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        }
        double cost = time + space * kRate;
        return {cost, L};
    }
    else
    {
        // cout << "In construct inner node: findData.size():" << findData.size() << "\tinsertData.size():" << insertData.size() << "\tkMaxKeyNum:" << kMaxKeyNum << endl;
        double OptimalValue = DBL_MAX;
        int space;
        int c;
        int optimalChildNumber = 32, optimalType = 0;
        int tmpIdx;
        for (int k = 16; k < findData.size();)
        {
            c = k;
            if (k < 4096)
                k *= 2;
            else if (k < 40960)
                k += 8192;
            else
                k += 65536;
            // cout << "c:" << c << "\tk:" << k << "\tsize:" << findData.size() << endl;
            if (512 * c < findData.size())
                continue;
            for (int type = 0; type < 4; type++)
            {
                // cout << "type:" << type << endl;
                switch (type)
                {
                case 0:
                {
                    space = 4 * c + 24;
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

                    double RootCost = time + kRate * space;
                    // cout << "now max child:" << c << endl;
                    for (int i = 0; i < c; i++)
                    {
                        pair<double, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);  // construct a leaf node
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace); // construct an inner node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                        tmpLRVec[idx].child.push_back(res.second);
                        RootCost += res.first;
                    }
                    if (RootCost < OptimalValue)
                    {
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                    }
                    // cout << "Rootcost:" << RootCost << endl;

                    vector<vector<pair<double, double>>>().swap(subFindData);
                    vector<vector<pair<double, double>>>().swap(subInsertData);
                    vector<vector<int>>().swap(subReadCnt);
                    vector<vector<int>>().swap(subWriteCnt);
                    break;
                }
                case 1:
                {
                    space = 4 * c + 24;
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

                    double RootCost = time + kRate * space;
                    // cout << "now max child:" << c << endl;
                    for (int i = 0; i < c; i++)
                    {
                        pair<double, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace); // construct an inner node
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);  // construct a leaf node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                        tmpNNVec[idx].child.push_back(res.second);
                        RootCost += res.first;
                    }
                    if (RootCost < OptimalValue)
                    {
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                    }
                    // cout << "Rootcost:" << RootCost << endl;

                    vector<vector<pair<double, double>>>().swap(subFindData);
                    vector<vector<pair<double, double>>>().swap(subInsertData);
                    vector<vector<int>>().swap(subReadCnt);
                    vector<vector<int>>().swap(subWriteCnt);
                    break;
                }
                case 2:
                {
                    space = 4 * c + 24;
                    if (space > maxSpace)
                        continue;
                    tmpHisVec.push_back(HisType(c));
                    int idx = tmpHisVec.size() - 1;
                    tmpHisVec[idx].model.Train(findData, c);
                    // cout<<"train over"<<endl;

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
                    // cout<<"insert over"<<endl;

                    // only record the time of inner node using cost model
                    double time = 12.4141 * 1e-9 * (findData.size() + insertData.size());

                    double RootCost = time + kRate * space;
                    // cout << "now max child:" << c << endl;
                    for (int i = 0; i < c; i++)
                    {
                        pair<double, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace); // construct an inner node
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);  // construct a leaf node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                        tmpHisVec[idx].child.push_back(res.second);
                        RootCost += res.first;
                    }
                    if (RootCost < OptimalValue)
                    {
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                    }

                    // cout << "Rootcost:" << RootCost << endl;
                    vector<vector<pair<double, double>>>().swap(subFindData);
                    vector<vector<pair<double, double>>>().swap(subInsertData);
                    vector<vector<int>>().swap(subReadCnt);
                    vector<vector<int>>().swap(subWriteCnt);
                    break;
                }
                case 3:
                {
                    space = 4 * c + 24;
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
                    double time = log(c) / log(2) * 1e-9 * (findData.size() + insertData.size());

                    double RootCost = time + kRate * space;
                    // cout << "now max child:" << c << endl;
                    for (int i = 0; i < c; i++)
                    {
                        pair<double, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace); // construct an inner node
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);  // construct a leaf node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                        tmpBSVec[idx].child.push_back(res.second);
                        RootCost += res.first;
                    }
                    if (RootCost < OptimalValue)
                    {
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                    }
                    // cout << "Rootcost:" << RootCost << endl;

                    vector<vector<pair<double, double>>>().swap(subFindData);
                    vector<vector<pair<double, double>>>().swap(subInsertData);
                    vector<vector<int>>().swap(subReadCnt);
                    vector<vector<int>>().swap(subWriteCnt);
                    break;
                }
                }
            }
        }
        // cout << "start store" << endl;
        int actualIdx = storeOptimalNode(optimalType, tmpIdx, findData, insertData);
        // cout << "optimalType:" << optimalType << "\tOptimalValue:" << OptimalValue << "\ttmpIdx:" << tmpIdx << "\tactualIdx:" << actualIdx << "\toptimalChildNumber:" << optimalChildNumber << endl;
        // cout << "LRVec size:" << LRVector.size() << endl;
        // cout << "NNVec size:" << NNVector.size() << endl;
        // cout << "HisVec size:" << HisVector.size() << endl;
        // cout << "BinVec size:" << BSVector.size() << endl;
        // cout << "Array size:" << ArrayVector.size() << endl;
        // cout << "GA size:" << GAVector.size() << endl;
        return {OptimalValue, (optimalType << 28) + actualIdx};
    }
}

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

    auto res = ChooseRoot(findDataset, kMaxSpace);
    int childNum = res.second;
    int rootType = res.first;
    cout << "Construction of the root node has been completed!" << endl;
    cout << "The optimal value of root is: " << res.first << "\tthe optimal child number is: " << res.second << endl;
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
            if (subFindData[i].size() + subInsertData[i].size() >= kMaxKeyNum)
                cout << "construct child " << i << ":" << endl;
            pair<double, int> resChild;
            if ((subFindData[i].size() + subInsertData[i].size()) >= kMaxKeyNum)
            {
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace); // construct an inner node
                // cout << "inner node over! res:" << res0.first << "\t" << res.second << endl;
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace); // construct a leaf node
                // cout << "leaf node over! res:" << res0.first << "\t" << res.second << endl;
                if (res0.first > res1.first)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);
            int idx = resChild.second;
            if (subFindData[i].size() + subInsertData[i].size() < kMaxKeyNum)
            {
                idx += storeOptimalNode(resChild.second >> 28, 0, subFindData[i], subInsertData[i]);
            }
            LRVector[0].child.push_back(idx);
            totalCost += resChild.first;
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
            pair<double, int> resChild;
            if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
            {
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);  // construct a leaf node
                if (res0.first > res1.first)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);
            int idx = resChild.second;
            if (subFindData[i].size() + subInsertData[i].size() < kMaxKeyNum)
            {
                idx += storeOptimalNode(resChild.second >> 28, 0, subFindData[i], subInsertData[i]);
            }
            NNVector[0].child.push_back(idx);
            totalCost += resChild.first;
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
            if (subFindData[i].size() + subInsertData[i].size() >= kMaxKeyNum)
                cout << "construct child " << i << ":" << endl;

            pair<double, int> resChild;
            if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
            {
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);  // construct a leaf node
                if (res0.first > res1.first)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);
            int idx = resChild.second;
            if (subFindData[i].size() + subInsertData[i].size() < kMaxKeyNum)
            {
                idx += storeOptimalNode(resChild.second >> 28, 0, subFindData[i], subInsertData[i]);
            }
            HisVector[0].child.push_back(idx);
            totalCost += resChild.first;
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
            if (subFindData[i].size() + subInsertData[i].size() >= kMaxKeyNum)
                cout << "construct child " << i << ":" << endl;

            pair<double, int> resChild;
            if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
            {
                auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace); // construct an inner node
                auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);  // construct a leaf node
                if (res0.first > res1.first)
                    resChild = res1;
                else
                    resChild = res0;
            }
            else
                resChild = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);
            int idx = resChild.second;
            if (subFindData[i].size() + subInsertData[i].size() < kMaxKeyNum)
            {
                idx += storeOptimalNode(resChild.second >> 28, 0, subFindData[i], subInsertData[i]);
            }
            BSVector[0].child.push_back(idx);
            totalCost += resChild.first;
        }
    }
    break;
    }
    cout << "total cost: " << totalCost << endl;
    return rootType;
}

#endif