#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "params.h"
#include "innerNodeType/bin_type.h"
#include "innerNodeType/his_type.h"
#include "innerNodeType/lr_type.h"
#include "innerNodeType/nn_type.h"
#include "leafNodeType/ga_type.h"
#include "leafNodeType/array_type.h"
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

extern int kMaxSpace;
extern int kMaxKeyNum;
extern double kRate;
extern const double kReadWriteRate;

pair<int, int> ChooseRoot(const vector<pair<double, double>> &dataset, const int maxSpace)
{
    double OptimalValue = DBL_MAX;

    // vector<int> candidateChildNum = {2000, 3000, 3907, 5000, 7500, 10000, 12500, 15000, 17500, 20000, 25000, 50000, 60000, 70125, 80000, 90000, 100000, 200000, 350000};
    vector<int> candidateChildNum = {200000};

    int space;
    int c;
    int optimalChildNumber, optimalType;
    cout << "Choose root!" << endl;
    for (int i = 0; i < candidateChildNum.size(); i++)
    {
        c = candidateChildNum[i];
        if (c * 512 < dataset.size())
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
                space = 20 * c + 20;
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
            // cout << "tmp root type:" << type << "\troot time:" << time << "\tEntropy:" << entropy << "\tratio: " << ratio << endl;
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

void deleteNode(int optimalType, int optimalChildNumber, int tmpIdx)
{
    switch (optimalType)
    {
    case 0:
    {
        int a, b, c;
        for (int i = 0; i < optimalChildNumber; i++)
        {
            a = LRVector[tmpIdx].child[i]; // content
            b = a >> 28;                   // type
            c = a & 0x0FFFFFFF;            // idx
            switch (b)
            {
            case 4:
                ArrayVector.erase(ArrayVector.begin() + c);
                break;
            case 5:
                GAVector.erase(GAVector.begin() + c);
                break;
            }
        }
        LRVector.erase(LRVector.begin() + tmpIdx);
        break;
    }
    case 1:
    {
        int a, b, c;
        for (int i = 0; i < optimalChildNumber; i++)
        {
            a = NNVector[tmpIdx].child[i]; // content
            b = a >> 28;                   // type
            c = a & 0x0FFFFFFF;            // idx
            switch (b)
            {
            case 4:
                ArrayVector.erase(ArrayVector.begin() + c);
                break;
            case 5:
                GAVector.erase(GAVector.begin() + c);
                break;
            }
        }
        NNVector.erase(NNVector.begin() + tmpIdx);
        break;
    }
    case 2:
    {
        int a, b, c;
        for (int i = 0; i < optimalChildNumber; i++)
        {
            a = HisVector[tmpIdx].child[i]; // content
            b = a >> 28;                    // type
            c = a & 0x0FFFFFFF;             // idx
            switch (b)
            {
            case 4:
                ArrayVector.erase(ArrayVector.begin() + c);
                break;
            case 5:
                GAVector.erase(GAVector.begin() + c);
                break;
            }
        }
        HisVector.erase(HisVector.begin() + tmpIdx);
        break;
    }
    case 4:
    {
        int a, b, c;
        for (int i = 0; i < optimalChildNumber; i++)
        {
            a = BSVector[tmpIdx].child[i]; // content
            b = a >> 28;                   // type
            c = a & 0x0FFFFFFF;            // idx
            switch (b)
            {
            case 4:
                ArrayVector.erase(ArrayVector.begin() + c);
                break;
            case 5:
                GAVector.erase(GAVector.begin() + c);
                break;
            }
        }
        BSVector.erase(BSVector.begin() + tmpIdx);
        break;
    }
    }
}

pair<double, int> Construct(const vector<pair<double, double>> &findData, const vector<int> &readCnt, const vector<pair<double, double>> &insertData, const vector<int> &writeCnt, int maxSpace)
{
    // construct a leaf node
    if (findData.size() + insertData.size() < kMaxKeyNum)
    {
        // cout << "In construct leaf node: findData.size():" << findData.size() << "\tinsertData.size():" << insertData.size() << "\tkMaxKeyNum:" << kMaxKeyNum << endl;
        int L; // stored in the child vector of the upper node
        double space;
        int idx, type;
        int read = 0, write = 0;
        for (int i = 0; i < readCnt.size(); i++)
            read += readCnt[i];
        for (int i = 0; i < writeCnt.size(); i++)
            write += writeCnt[i];
        cout << "In leaf: readTime:" << read << "\twriteTime:" << write << endl;
        if ((read == 0 && write == 0) || float(read) / (float(read + write)) >= kReadWriteRate)
        {
            // choose an array node as the leaf node
            ArrayVector.push_back(ArrayType(kMaxKeyNum));
            idx = ArrayVector.size() - 1;
            ArrayVector[idx].SetDataset(findData);
            L = 0x40000000 + idx;
            space = 36 + findData.size() * 4;
            type = 4;
        }
        else
        {
            // choose a gapped array node as the leaf node
            GAVector.push_back(GappedArrayType(kMaxKeyNum));
            idx = GAVector.size() - 1;
            GAVector[idx].SetDataset(findData);
            L = 0x50000000 + idx;
            space = 44 + findData.size() * 4 / kDensity;
            type = 5;
        }
        cout << "Now size:" << endl;
        cout << "ArrayVector: " << ArrayVector.size() << endl;
        cout << "GAVector: " << GAVector.size() << endl;
        cout << "***************************************************" << endl;
        chrono::_V2::system_clock::time_point s, e;
        double time;
        s = chrono::system_clock::now();
        for (int i = 0; i < findData.size(); i++)
            LeafNodeFind(type, idx, findData[i].first);
        for (int i = 0; i < insertData.size(); i++)
            LeafNodeInsert(type, idx, insertData[i]);
        e = chrono::system_clock::now();
        time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        // cout << "leaf node time:" << time << endl;
        double cost = time + space * kRate;
        return {cost, L};
    }
    else
    {
        cout << "In construct inner node: findData.size():" << findData.size() << "\tinsertData.size():" << insertData.size() << "\tkMaxKeyNum:" << kMaxKeyNum << endl;
        double OptimalValue = DBL_MAX;

        // vector<int> candidateChildNum = {32, 64, 128, 256, 512, 2000, 3000, 3907, 5000, 7500, 10000, 12500, 15000, 17500, 20000, 25000, 50000, 60000, 70125, 80000, 90000, 100000, 250000, 350000, 500000, 750000, 1000000};
        vector<int> candidateChildNum = {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 8192 * 2, 8192 * 4, 8192 * 8, 8192 * 16, 8192 * 32, 8192 * 64, 8192 * 128};

        int space;
        int c;
        int optimalChildNumber = 32, optimalType = 0;
        int tmpIdx;
        for (int k = 0; k < candidateChildNum.size(); k++)
        {
            c = candidateChildNum[k];
            if (4 * c > findData.size() || 10 * c < findData.size())
                break;
            for (int type = 0; type < 1; type++)
            {
                // cout << "now type:" << type << "\tnow childNum:" << c << endl;
                switch (type)
                {
                case 0:
                {
                    space = 4 * c + 24;
                    if (space > maxSpace)
                        continue;
                    LRVector.push_back(LRType(c));
                    int idx = LRVector.size() - 1;
                    LRVector[idx].model.Train(findData, c);
                    for (int i = 0; i < c; i++)
                    {
                        LRVector[idx].child.push_back(0x40000000 + i);
                    }

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
                        int p = LRVector[idx].model.Predict(findData[i].first);
                        subFindData[p].push_back(findData[i]);
                        subReadCnt[p].push_back(readCnt[i]);
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = LRVector[idx].model.Predict(insertData[i].first);
                        subInsertData[p].push_back(insertData[i]);
                        subWriteCnt[p].push_back(writeCnt[i]);
                    }
                    // cout << "now test inner node time, childNum:" << LRVector[idx].child.size() << "\tidx:" << idx << endl;

                    // only record the time of inner node
                    chrono::_V2::system_clock::time_point s, e;
                    s = chrono::system_clock::now();
                    for (int i = 0; i < findData.size(); i++)
                        InnerNodeTime(idx, type, findData[i].first);
                    for (int i = 0; i < insertData.size(); i++)
                        InnerNodeTime(idx, type, insertData[i].first);
                    e = chrono::system_clock::now();
                    double time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

                    // double RootCost = time + kRate * space;
                    double RootCost = time;
                    for (int i = 0; i < c; i++)
                    {
                        auto res = Construct(subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                        LRVector[idx].child.push_back(res.second);
                        RootCost += res.first;
                    }
                    cout << "Now size:" << endl;
                    cout << "LRVector: " << LRVector.size() << endl;
                    cout << "NNVector: " << NNVector.size() << endl;
                    cout << "HisVector: " << HisVector.size() << endl;
                    cout << "BinVector: " << BSVector.size() << endl;
                    cout << "ArrayVector: " << ArrayVector.size() << endl;
                    cout << "GAVector: " << GAVector.size() << endl;
                    if (RootCost < OptimalValue)
                    {
                        cout << "find better!" << endl;
                        if (OptimalValue != DBL_MAX)
                        {
                            cout << "not dblmax" << endl;
                            deleteNode(optimalType, optimalChildNumber, tmpIdx);
                        }
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                    }
                    else
                        deleteNode(type, c, idx);
                    cout << "Now size:" << endl;
                    cout << "LRVector: " << LRVector.size() << endl;
                    cout << "NNVector: " << NNVector.size() << endl;
                    cout << "HisVector: " << HisVector.size() << endl;
                    cout << "BinVector: " << BSVector.size() << endl;
                    cout << "ArrayVector: " << ArrayVector.size() << endl;
                    cout << "GAVector: " << GAVector.size() << endl;

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
                    NNVector.push_back(NNType(c));
                    int idx = NNVector.size() - 1;
                    NNVector[idx].model.Train(findData, c);
                    for (int i = 0; i < c; i++)
                    {
                        NNVector[idx].child.push_back(0x40000000 + i);
                    }

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
                        int p = NNVector[idx].model.Predict(findData[i].first);
                        subFindData[p].push_back(findData[i]);
                        subReadCnt[p].push_back(readCnt[i]);
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = NNVector[idx].model.Predict(insertData[i].first);
                        subInsertData[p].push_back(insertData[i]);
                        subWriteCnt[p].push_back(writeCnt[i]);
                    }

                    // only record the time of inner node
                    chrono::_V2::system_clock::time_point s, e;
                    s = chrono::system_clock::now();
                    for (int i = 0; i < findData.size(); i++)
                        InnerNodeTime(idx, type, findData[i].first);
                    for (int i = 0; i < insertData.size(); i++)
                        InnerNodeTime(idx, type, insertData[i].first);
                    e = chrono::system_clock::now();
                    double time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

                    double RootCost = time + kRate * space;
                    for (int i = 0; i < c; i++)
                    {
                        auto res = Construct(subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                        NNVector[idx].child.push_back(res.second);
                        RootCost += res.first;
                    }
                    if (RootCost < OptimalValue)
                    {
                        if (OptimalValue != DBL_MAX)
                            deleteNode(optimalType, optimalChildNumber, tmpIdx);
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                    }
                    else
                        deleteNode(type, c, idx);

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
                    HisVector.push_back(HisType(c));
                    int idx = HisVector.size() - 1;
                    HisVector[idx].model.Train(findData, c);
                    for (int i = 0; i < c; i++)
                    {
                        HisVector[idx].child.push_back(0x40000000 + i);
                    }

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
                        int p = HisVector[idx].model.Predict(findData[i].first);
                        subFindData[p].push_back(findData[i]);
                        subReadCnt[p].push_back(readCnt[i]);
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = HisVector[idx].model.Predict(insertData[i].first);
                        subInsertData[p].push_back(insertData[i]);
                        subWriteCnt[p].push_back(writeCnt[i]);
                    }

                    // only record the time of inner node
                    chrono::_V2::system_clock::time_point s, e;
                    s = chrono::system_clock::now();
                    for (int i = 0; i < findData.size(); i++)
                        InnerNodeTime(idx, type, findData[i].first);
                    for (int i = 0; i < insertData.size(); i++)
                        InnerNodeTime(idx, type, insertData[i].first);
                    e = chrono::system_clock::now();
                    double time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

                    double RootCost = time + kRate * space;
                    for (int i = 0; i < c; i++)
                    {
                        auto res = Construct(subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                        HisVector[idx].child.push_back(res.second);
                        RootCost += res.first;
                    }
                    if (RootCost < OptimalValue)
                    {
                        if (OptimalValue != DBL_MAX)
                            deleteNode(optimalType, optimalChildNumber, tmpIdx);
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                    }
                    else
                        deleteNode(type, c, idx);

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
                    BSVector.push_back(BSType(c));
                    int idx = BSVector.size() - 1;
                    BSVector[idx].model.Train(findData, c);
                    for (int i = 0; i < c; i++)
                    {
                        BSVector[idx].child.push_back(0x40000000 + i);
                    }

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
                        int p = BSVector[idx].model.Predict(findData[i].first);
                        subFindData[p].push_back(findData[i]);
                        subReadCnt[p].push_back(readCnt[i]);
                    }
                    for (int i = 0; i < insertData.size(); i++)
                    {
                        int p = BSVector[idx].model.Predict(insertData[i].first);
                        subInsertData[p].push_back(insertData[i]);
                        subWriteCnt[p].push_back(writeCnt[i]);
                    }

                    // only record the time of inner node
                    chrono::_V2::system_clock::time_point s, e;
                    s = chrono::system_clock::now();
                    for (int i = 0; i < findData.size(); i++)
                        InnerNodeTime(idx, type, findData[i].first);
                    for (int i = 0; i < insertData.size(); i++)
                        InnerNodeTime(idx, type, insertData[i].first);
                    e = chrono::system_clock::now();
                    double time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

                    double RootCost = time + kRate * space;
                    for (int i = 0; i < c; i++)
                    {
                        auto res = Construct(subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                        BSVector[idx].child.push_back(res.second);
                        RootCost += res.first;
                    }
                    if (RootCost < OptimalValue)
                    {
                        if (OptimalValue != DBL_MAX)
                            deleteNode(optimalType, optimalChildNumber, tmpIdx);
                        tmpIdx = idx;
                        optimalChildNumber = c;
                        optimalType = type;
                        OptimalValue = RootCost;
                    }
                    else
                        deleteNode(type, c, idx);

                    vector<vector<pair<double, double>>>().swap(subFindData);
                    vector<vector<pair<double, double>>>().swap(subInsertData);
                    vector<vector<int>>().swap(subReadCnt);
                    vector<vector<int>>().swap(subWriteCnt);
                    break;
                }
                }
            }
        }
        cout << "A new inner node is constructed! type:" << optimalType << "\tchildNum:" << optimalChildNumber << "\toptimal value:" << OptimalValue << "\ttmpIdx:" << tmpIdx << endl;
        cout << "Now size:" << endl;
        cout << "LRVector: " << LRVector.size() << endl;
        cout << "NNVector: " << NNVector.size() << endl;
        cout << "HisVector: " << HisVector.size() << endl;
        cout << "BinVector: " << BSVector.size() << endl;
        cout << "ArrayVector: " << ArrayVector.size() << endl;
        cout << "GAVector: " << GAVector.size() << endl;
        return {OptimalValue, (optimalType << 28) + tmpIdx};
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

    auto res = ChooseRoot(findDataset, kMaxSpace);
    int childNum = res.second;
    int rootType = res.first;
    cout << "Construction of the root node has been completed!" << endl;
    cout << "The optimal value is: " << res.first << "\tthe optimal child number is: " << res.second << endl;
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
    cout << "Now size:" << endl;
    cout << "LRVector: " << LRVector.size() << endl;
    cout << "NNVector: " << NNVector.size() << endl;
    cout << "HisVector: " << HisVector.size() << endl;
    cout << "BinVector: " << BSVector.size() << endl;
    cout << "ArrayVector: " << ArrayVector.size() << endl;
    cout << "GAVector: " << GAVector.size() << endl;

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
            // if (subFindData[i].size() + subInsertData[i].size() >= kMaxKeyNum)
            cout << "construct child " << i << ":" << endl;
            auto resChild = Construct(subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);
            LRVector[0].child.push_back(resChild.second);
            totalCost += resChild.first;
        }
    }
    break;
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
            if (subFindData[i].size() + subInsertData[i].size() >= kMaxKeyNum)
                cout << "construct child " << i << ":" << endl;
            auto resChild = Construct(subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);
            NNVector[0].child.push_back(resChild.second);
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
            auto resChild = Construct(subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);
            HisVector[0].child.push_back(resChild.second);
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
            auto resChild = Construct(subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], kMaxSpace);
            BSVector[0].child.push_back(resChild.second);
            totalCost += resChild.first;
        }
    }
    break;
    }
    cout << "total cost: " << totalCost << endl;
    return rootType;
}

#endif