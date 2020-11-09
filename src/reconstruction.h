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

    vector<int> candidateChildNum = {2000, 3000, 3907, 5000, 7500, 10000, 12500, 15000, 17500, 20000, 25000, 50000, 60000, 70125, 80000, 90000, 100000, 250000, 350000, 500000, 750000, 1000000};

    int space;
    int c;
    int optimalChildNumber, optimalType;
    for (int i = 0; i < candidateChildNum.size(); i++)
    {
        c = candidateChildNum[i];
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
                InnerNodeTime(type, dataset[i].first);
            e = chrono::system_clock::now();
            double time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
            cout << "Average time:" << time / (float)dataset.size() << endl;

            auto entropy = GetEntropy(dataset.size());
            cout << "Entropy:" << entropy << endl;
            double ratio = time / entropy;
            if (ratio < OptimalValue)
            {
                optimalChildNumber = c;
                optimalType = type;
                OptimalValue = ratio;
            }
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
        int L; // stored in the child vector of the upper node
        double space;
        int idx, type;
        int read = 0, write = 0;
        for (int i = 0; i < readCnt.size(); i++)
            read += readCnt[i];
        for (int i = 0; i < writeCnt.size(); i++)
            write += writeCnt[i];
        if (float(read) / (float(read + write)) >= kReadWriteRate)
        {
            // choose an array node as the leaf node
            ArrayVector.push_back(ArrayType(kThreshold));
            idx = ArrayVector.size() - 1;
            ArrayVector[idx].SetDataset(findData);
            L = 0x40000000 + idx;
            space = 36 + findData.size() * 4;
            type = 4;
        }
        else
        {
            // choose a gapped array node as the leaf node
            GAVector.push_back(GappedArrayType(kThreshold));
            idx = GAVector.size() - 1;
            GAVector[idx].SetDataset(findData);
            L = 0x50000000 + idx;
            space = 44 + findData.size() * 4 / kDensity;
            type = 5;
        }
        chrono::_V2::system_clock::time_point s, e;
        double time;
        s = chrono::system_clock::now();
        for (int i = 0; i < findData.size(); i++)
            LeafNodeFind(type, idx, findData[i].first);
        for (int i = 0; i < insertData.size(); i++)
            LeafNodeInsert(type, idx, insertData[i]);
        e = chrono::system_clock::now();
        time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;
        cout << "leaf node time:" << time << endl;
        double cost = time + space * kRate;
        return {cost, L};
    }
    else
    {
        double OptimalValue = DBL_MAX;

        vector<int> candidateChildNum = {2000, 3000, 3907, 5000, 7500, 10000, 12500, 15000, 17500, 20000, 25000, 50000, 60000, 70125, 80000, 90000, 100000, 250000, 350000, 500000, 750000, 1000000};

        int space;
        int c;
        int optimalChildNumber, optimalType;
        int tmpIdx;
        for (int k = 0; k < candidateChildNum.size(); k++)
        {
            c = candidateChildNum[k];
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
                    int idx = LRVector.size() - 1;
                    LRVector[idx].model.Train(findData, c);

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

                    // only record the time of inner node
                    chrono::_V2::system_clock::time_point s, e;
                    s = chrono::system_clock::now();
                    for (int i = 0; i < findData.size(); i++)
                        InnerNodeTime(type, findData[i].first);
                    for (int i = 0; i < insertData.size(); i++)
                        InnerNodeTime(type, insertData[i].first);
                    e = chrono::system_clock::now();
                    double time = double(chrono::duration_cast<chrono::nanoseconds>(e - s).count()) / chrono::nanoseconds::period::den;

                    double RootCost = time + kRate * space;
                    for (int i = 0; i < c; i++)
                    {
                        auto res = Construct(subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i], maxSpace);
                        LRVector[idx].child.push_back(res.second);
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
                        InnerNodeTime(type, findData[i].first);
                    for (int i = 0; i < insertData.size(); i++)
                        InnerNodeTime(type, insertData[i].first);
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
                        InnerNodeTime(type, findData[i].first);
                    for (int i = 0; i < insertData.size(); i++)
                        InnerNodeTime(type, insertData[i].first);
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
                        InnerNodeTime(type, findData[i].first);
                    for (int i = 0; i < insertData.size(); i++)
                        InnerNodeTime(type, insertData[i].first);
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
                    break;
                }
                }
            }
        }
    }
}

// main function of construction
// findDataset: the dataset used to initialize the index
// insertDataset: the dataset to be inserted into the index
// readCnt: the number of READ corresponding to each key
// writeCnt: the number of WRITE corresponding to each key
void Construction(const vector<pair<double, double>> &findDataset, const vector<int> &readCnt, const vector<pair<double, double>> &insertDataset, const vector<int> &writeCnt)
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
    cout << "Construction of the root node has been completed!" << endl;
    cout << "The optimal value is: " << res.first << ",\tthe optimal child number is: " << res.second << endl;
}

#endif