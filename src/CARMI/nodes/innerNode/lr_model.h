#ifndef LR_MODEL_H
#define LR_MODEL_H

#include "../../carmi.h"
#include <vector>
using namespace std;

inline void CARMI::initLR(LRModel *lr, const vector<pair<double, double>> &dataset)
{
    int childNumber = lr->flagNumber & 0x00FFFFFF;
    lr->childLeft = allocateChildMemory(childNumber);
    if (dataset.size() == 0)
        return;

    lr->Train(dataset);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int p = lr->Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
    }

    switch (kLeafNodeID)
    {
    case 0:
        for (int i = 0; i < childNumber; i++)
        {
            ArrayType tmp(kThreshold);
            initArray(&tmp, perSubDataset[i], kMaxKeyNum);
            entireChild[lr->childLeft + i].array = tmp;
        }
        break;
    case 1:
        for (int i = 0; i < childNumber; i++)
        {
            GappedArrayType tmp(kThreshold);
            initGA(&tmp, perSubDataset[i], kMaxKeyNum);
            entireChild[lr->childLeft + i].ga = tmp;
        }
        break;
    }
}

inline void CARMI::Train(LRModel *lr, const int left, const int size)
{
    if (size == 0)
        return;
    int childNumber = lr->flagNumber & 0x00FFFFFF;
    int end = left + size;
    double maxValue;
    for (int i = left; i < end; i++)
    {
        lr->minValue = initDataset[i].first;
        break;
    }
    for (int i = end - 1; i >= left; i--)
    {
        maxValue = initDataset[i].first;
        break;
    }
    lr->divisor = float(maxValue - lr->minValue) / 6;

    vector<double> index;
    int j = 0;
    int cnt = 0;
    for (int i = left; i < end; i++)
    {
        int idx = float(initDataset[i].first - lr->minValue) / lr->divisor;
        if (idx < 0)
            idx = 0;
        else if (idx >= 6)
            idx = 5;
        if (idx != cnt)
        {
            cnt = idx;
            j = 0;
        }
        index.push_back(double(j));
        j++;
    }

    int i = left;
    cnt = 0;
    for (int k = 1; k <= 6; k++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        for (; i < end - 1; i++)
        {
            if (float(initDataset[i].first - lr->minValue) / lr->divisor >= k)
                break;
            cnt++;
            t1 += initDataset[i].first * initDataset[i].first;
            t2 += initDataset[i].first;
            t3 += initDataset[i].first * index[i - left];
            t4 += index[i - left];
        }
        t3 /= cnt;
        t4 /= cnt;
        if (t1 * cnt - t2 * t2 != 0)
        {
            auto theta1 = (t3 * cnt - t2 * t4) / (t1 * cnt - t2 * t2);
            auto theta2 = (t1 * t4 - t2 * t3) / (t1 * cnt - t2 * t2);
            theta1 *= childNumber;
            theta2 *= childNumber;
            lr->theta[k - 1] = {theta1, theta2};
        }
        else
        {
            lr->theta[k - 1] = {childNumber, 0};
        }
    }
}

inline void LRModel::Train(const vector<pair<double, double>> &dataset)
{
    int actualSize = 0;
    int childNumber = flagNumber & 0x00FFFFFF;
    double maxValue;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            minValue = dataset[i].first;
            break;
        }
    }
    for (int i = dataset.size() - 1; i >= 0; i--)
    {
        if (dataset[i].first != -1)
        {
            maxValue = dataset[i].first;
            break;
        }
    }
    divisor = float(maxValue - minValue) / 6;

    vector<double> index;
    int j = 0, cnt = 0;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            actualSize++;
            int idx = float(dataset[i].first - minValue) / divisor;
            if (idx < 0)
                idx = 0;
            else if (idx >= 6)
                idx = 5;
            if (idx != cnt)
            {
                cnt = idx;
                j = 0;
            }
        }
        index.push_back(double(j));
        j++;
    }
    if (actualSize == 0)
        return;

    int i = 0;
    cnt = 0;
    for (int k = 1; k <= 6; k++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        for (; i < dataset.size() - 1; i++)
        {
            if (dataset[i].first != -1)
            {
                if (float(dataset[i].first - minValue) / divisor >= k)
                    break;
                cnt++;
                t1 += dataset[i].first * dataset[i].first;
                t2 += dataset[i].first;
                t3 += dataset[i].first * index[i];
                t4 += index[i];
            }
        }
        t3 /= cnt;
        t4 /= cnt;
        if (t1 * cnt - t2 * t2 != 0)
        {
            auto theta1 = (t3 * cnt - t2 * t4) / (t1 * cnt - t2 * t2);
            auto theta2 = (t1 * t4 - t2 * t3) / (t1 * cnt - t2 * t2);
            theta1 *= childNumber;
            theta2 *= childNumber;
            theta[k - 1] = {theta1, theta2};
        }
        else
        {
            theta[k - 1] = {childNumber, 0};
        }
    }
}

inline int LRModel::Predict(double key)
{
    int idx = float(key - minValue) / divisor;
    if (idx < 0)
        idx = 0;
    else if (idx >= 6)
        idx = 5;
    // return the predicted idx in the children
    int p = theta[idx].first * key + theta[idx].second;
    int bound = (flagNumber & 0x00FFFFFF) / 6;
    int left = bound * idx;
    if (p < left)
        p = left;
    else if (p >= left + bound)
        p = left + bound - 1;
    return p;
}
#endif