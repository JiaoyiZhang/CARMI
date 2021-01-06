#ifndef HIS_MODEL_H
#define HIS_MODEL_H

#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include "../baseNode.h"
using namespace std;

extern vector<BaseNode> entireChild;

inline void HisModel::Initialize(const vector<pair<double, double>> &dataset)
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

inline void HisModel::Train(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
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
    divisor = float(maxValue - minValue) / childNumber;
    vector<int> table;
    for (int i = 0; i < childNumber; i++)
        table.push_back(0);
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            int idx = float(dataset[i].first - minValue) / divisor;
            idx = min(idx, int(table.size()) - 1);
            table[idx]++;
        }
    }
    int cnt = 0;
    int nowSize = 0;
    int avg = dataset.size() / childNumber;
    for (int i = 0; i < table.size(); i++)
    {
        nowSize += table[i];
        if (table[i] >= avg || nowSize >= avg)
        {
            cnt++;
            nowSize = 0;
        }
        if (cnt >= childNumber / 2)
            cnt = childNumber / 2 - 1;
        table[i] = cnt;
    }

    int i = 0;
    int idx0 = 0, idx1 = 1;
    vector<unsigned int> table00; // 2c/32*8 = c/2 Byte
    vector<unsigned int> table11; // c/2 Byte
    for (; i < childNumber; i += 32)
    {
        //  & 0x0FFFFFFF;
        unsigned int start_idx = table[i];
        int tmp = 0;
        for (int j = i; j < i + 32; j++)
        {
            if (j - i == 16)
                start_idx = table[i + 16];
            if (j >= childNumber)
            {
                while (j < i + 32)
                {
                    tmp = tmp << 1;
                    j++;
                }
                table11.push_back(int(table[i]) << 16);
                if (i + 16 < childNumber)
                    table11[table11.size() - 1] = (int(table[i]) << 16) + int(table[i + 16]);
                table00.push_back(tmp);
                for (int t = 0; t < table00.size(); t++)
                {
                    table0[t] = table00[t];
                    table1[t] = table11[t];
                }
                return;
            }
            int diff = int(table[j]) - start_idx;
            tmp = (tmp << 1) + diff;
            if (diff > 0)
                start_idx += diff;
        }
        start_idx = (int(table[i]) << 16) + int(table[i + 16]);
        table11.push_back(start_idx);
        table00.push_back(tmp);
    }
    for (int t = 0; t < table00.size(); t++)
    {
        table0[t] = table00[t];
        table1[t] = table11[t];
    }
}

inline int HisModel::Predict(double key)
{
    // return the idx in children
    int childNumber = flagNumber & 0x00FFFFFF;
    int idx = float(key - minValue) / divisor;
    if (idx < 0)
        idx = 0;
    else if (idx >= childNumber)
        idx = childNumber - 1;

    int base;
    int tmpIdx = idx / 32;
    if ((idx / 16) % 2 == 0)
    {
        base = table1[tmpIdx] >> 16;
        int j = idx % 16;
        int tmp = table0[tmpIdx] >> (31 - j);
        for (; j >= 0; j--)
        {
            base += tmp & 1;
            tmp = tmp >> 1;
        }
    }
    else
    {
        base = table1[tmpIdx] & 0x0000FFFF;
        int j = idx % 16;
        int tmp = table0[tmpIdx] >> (15 - j);
        for (; j >= 0; j--)
        {
            base += tmp & 1;
            tmp = tmp >> 1;
        }
    }
    return base;
}

#endif