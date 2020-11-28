#ifndef DP_H
#define DP_H

#include "../params.h"
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
#include "../function.h"
#include <float.h>
#include <vector>
using namespace std;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

extern vector<LRType> tmpLRVec;
extern vector<NNType> tmpNNVec;
extern vector<HisType> tmpHisVec;
extern vector<BSType> tmpBSVec;
extern vector<ArrayType> tmpArrayVec;
extern vector<GappedArrayType> tmpGAVec;

pair<pair<double, double>, int> Construct(bool isLeaf, const vector<pair<double, double>> &findData, const vector<int> &readCnt, const vector<pair<double, double>> &insertData, const vector<int> &writeCnt)
{
    // construct a leaf node
    if (isLeaf)
    {
        int L; // stored in the child vector of the upper node

        double OptimalValue = DBL_MAX;
        int optimalChildNumber = 32, optimalType = 0, optimalSpace = 0;
        int tmpIdx;

        int type;
        double space, time, cost;

        int read = 0, write = 0;
        for (int i = 0; i < readCnt.size(); i++)
            read += readCnt[i];
        for (int i = 0; i < writeCnt.size(); i++)
            write += writeCnt[i];
        // choose an array node as the leaf node
        tmpArrayVec.push_back(ArrayType(kMaxKeyNum));
        int idx = tmpArrayVec.size() - 1;

        type = 4;
        space = sizeof(ArrayType) / findData.size() + 16;
        time = read * 55 + write * 100; // TBD
        cost = time + space * kRate;
        if (cost <= OptimalValue)
        {
            tmpIdx = idx;
            optimalType = type;
            OptimalValue = cost;
            optimalSpace = space;
        }
        else
            tmpArrayVec.pop_back();

        // choose a gapped array node as the leaf node
        float Density[4] = {0.5, 0.7, 0.8, 0.9}; // data/capacity
        type = 5;
        for (int i = 0; i < 4; i++)
        {
            tmpGAVec.push_back(GappedArrayType(kMaxKeyNum));
            idx = tmpGAVec.size() - 1;
            tmpGAVec[idx].density = Density[i];
            auto tmp = GappedArrayType(kMaxKeyNum);
            space = sizeof(GappedArrayType) / findData.size() + 16 / tmpGAVec[idx].density;
            time = read * 75 + write * 10; // TBD
            cost = time + space * kRate;
            if (cost <= OptimalValue)
            {
                tmpIdx = idx;
                optimalType = type;
                OptimalValue = cost;
                optimalSpace = space;
            }
            else
                tmpGAVec.pop_back();
        }
        return {{OptimalValue, optimalSpace}, (optimalType << 28) + tmpIdx};
    }
    else
    {
        double OptimalValue = DBL_MAX;
        int space;
        int c;
        int optimalChildNumber = 32, optimalType = 0, optimalSpace = 0;
        int tmpIdx;
        for (int c = 16; c < findData.size();)
        {
            if (c < 4096)
                c *= 2;
            else if (c < 40960)
                c += 8192;
            else if (c <= 1000000)
                c += 65536;
            else
                c *= 2;
            if (512 * c < findData.size())
                continue;
            for (int type = 0; type < 4; type++)
            {
                switch (type)
                {
                case 0:
                {
                    space = (4 * c + sizeof(LRType));
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
                    double time = 8.1624; // ns
                    double RootCost = time + kRate * space / 1024 / 1024;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);  // construct a leaf node
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);
                        tmpLRVec[idx].child.push_back(res.second);
                        RootCost += res.first.first;
                        space += res.first.second;
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
                    double time = 20.2689; // ns

                    double RootCost = time + kRate * space / 1024 / 1024;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);  // construct a leaf node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);
                        tmpNNVec[idx].child.push_back(res.second);
                        RootCost += res.first.first;
                        space += res.first.second;
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
                    double time = 19.6543;

                    double RootCost = time + kRate * space / 1024 / 1024;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);  // construct a leaf node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);
                        tmpHisVec[idx].child.push_back(res.second);
                        RootCost += res.first.first;
                        space += res.first.second;
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
                    double time = 4 * log(c) / log(2);

                    double RootCost = time + kRate * space / 1024 / 1024;
                    for (int i = 0; i < c; i++)
                    {
                        pair<pair<double, double>, int> res;
                        if ((subFindData[i].size() + subInsertData[i].size()) > kMaxKeyNum)
                        {
                            auto res0 = Construct(false, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]); // construct an inner node
                            auto res1 = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);  // construct a leaf node
                            if (res0.first > res1.first)
                                res = res1;
                            else
                                res = res0;
                        }
                        else
                            res = Construct(true, subFindData[i], subReadCnt[i], subInsertData[i], subWriteCnt[i]);
                        tmpBSVec[idx].child.push_back(res.second);
                        RootCost += res.first.first;
                        space += res.first.second;
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

#endif // !DP_H