
#ifndef NN_MODEL_H
#define NN_MODEL_H

#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include "../baseNode.h"
using namespace std;

extern vector<BaseNode> entireChild;

inline void NNModel::Initialize(const vector<pair<double, double>> &dataset)
{
    int childNumber = flagNumber & 0x00FFFFFF;
    childLeft = allocateChildMemory(childNumber);
    if (dataset.size() == 0)
        return;

    Train(dataset);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int p = Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
    }

    switch (kLeafNodeID)
    {
    case 0:
        for (int i = 0; i < childNumber; i++)
        {
            ArrayType tmp(kThreshold);
            tmp.SetDataset(perSubDataset[i], kMaxKeyNum);
            entireChild[childLeft + i].array = tmp;
        }
        break;
    case 1:
        for (int i = 0; i < childNumber; i++)
        {
            GappedArrayType tmp(kThreshold);
            tmp.SetDataset(perSubDataset[i], kMaxKeyNum);
            entireChild[childLeft + i].ga = tmp;
        }
        break;
    }
}

inline void NNModel::Train(const vector<pair<double, double>> &dataset)
{
    int childNumber = flagNumber & 0x00FFFFFF;
    int length = childNumber - 1;
    int actualSize = 0;
    vector<double> index;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
            actualSize++;
        index.push_back(double(i) / double(dataset.size()));
    }
    if (actualSize == 0)
        return;

    int seg = dataset.size() / 3;
    int i = 0;
    int cnt = 0;
    for (int k = 1; k <= 3; k++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        int end = min(k * seg, int(dataset.size() - 1));
        if (dataset[end].first != -1)
            point[cnt++] = {dataset[end].first, length};
        else
            point[cnt++] = {dataset[end - 1].first, length};
        for (; i < end; i++)
        {
            if (dataset[i].first != -1)
            {
                t1 += dataset[i].first * dataset[i].first;
                t2 += dataset[i].first;
                t3 += dataset[i].first * index[i];
                t4 += index[i];
            }
        }
        auto theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
        auto theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);
        theta1 *= childNumber;
        theta2 *= childNumber;
        int pointIdx = theta1 * point[k - 1].first + theta2;
        if (pointIdx < 0)
            pointIdx = 0;
        else if (pointIdx > length)
            pointIdx = length;
        point[k - 1].second = pointIdx;
    }
}

inline int NNModel::Predict(double key)
{
    int s = 0;
    int e = 2;
    int mid;
    while (s < e)
    {
        mid = (s + e) / 2;
        if (point[mid].first < key)
            s = mid + 1;
        else
            e = mid;
    }
    // return the predicted idx in the children
    // int p = theta[e].first * key + theta[e].second;
    int p = 0;
    if (e == 0)
    {
        if (p < 0)
            p = 0;
        else if (p > point[e].second)
            p = point[e].second;
    }
    else
    {
        if (p < point[e - 1].second)
            p = point[e - 1].second;
        else if (p > point[e].second)
            p = point[e].second;
    }
    return p;
}

#endif