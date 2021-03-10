#ifndef HIS_MODEL_H
#define HIS_MODEL_H

#include "../../carmi.h"
#include <vector>
using namespace std;

inline void CARMI::initHis(HisModel *his, const vector<pair<double, double>> &dataset)
{
    int childNumber = his->flagNumber & 0x00FFFFFF;
    his->childLeft = allocateChildMemory(childNumber);
    if (dataset.size() == 0)
        return;

    his->Train(dataset);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int p = his->Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
    }

    switch (kLeafNodeID)
    {
    case 0:
        for (int i = 0; i < childNumber; i++)
        {
            ArrayType tmp(kThreshold);
            initArray(&tmp, perSubDataset[i], kMaxKeyNum);
            entireChild[his->childLeft + i].array = tmp;
        }
        break;
    case 1:
        for (int i = 0; i < childNumber; i++)
        {
            GappedArrayType tmp(kThreshold);
            initGA(&tmp, perSubDataset[i], kMaxKeyNum);
            entireChild[his->childLeft + i].ga = tmp;
        }
        break;
    }
}

inline void CARMI::Train(HisModel *his, const int left, const int size)
{
    if (size == 0)
        return;
    int childNumber = his->flagNumber & 0x00FFFFFF;
    double maxValue;
    int end = left + size;
    for (int i = left; i < end; i++)
    {
        his->minValue = initDataset[i].first;
        break;
    }
    for (int i = end - 1; i >= left; i--)
    {
        maxValue = initDataset[i].first;
        break;
    }
    his->divisor = float(maxValue - his->minValue) / childNumber;
    vector<float> table(childNumber, 0);
    int cnt = 0;
    while (cnt <= 1)
    {
        if (cnt == 1)
            his->divisor /= 10;
        cnt = 0;
        table = vector<float>(childNumber, 0);
        for (int i = left; i < end; i++)
        {
            int idx = float(initDataset[i].first - his->minValue) / his->divisor;
            idx = min(idx, int(table.size()) - 1);
            table[idx]++;
        }
        if (table[0] > 0)
            cnt++;
        table[0] = table[0] / size * childNumber;
        for (int i = 1; i < table.size(); i++)
        {
            if (table[i] > 0)
                cnt++;
            table[i] = table[i] / size * childNumber + table[i - 1];
        }
    }
    table[0] = round(table[0]);
    for (int i = 1; i < childNumber; i++)
    {
        table[i] = round(table[i]);
        if (table[i] - table[i - 1] > 1)
            table[i] = table[i - 1] + 1;
    }

    cnt = 0;
    for (int i = 0; i < childNumber; i += 16)
    {
        unsigned short start_idx = int(table[i]);
        unsigned short tmp = 0;
        for (int j = i; j < i + 16; j++)
        {
            if (j >= childNumber)
            {
                while (j < i + 16)
                {
                    tmp = tmp << 1;
                    j++;
                }
                his->Base[cnt] = table[i];
                his->Offset[cnt] = tmp;
                return;
            }
            unsigned short diff = int(table[j]) - start_idx;
            tmp = (tmp << 1) + diff;
#ifdef DEBUG
            if (diff > 1)
                cout << "diff wrong, diff:" << diff << endl;
#endif // DEBUG
            if (diff > 0)
                start_idx += diff;
        }
        his->Base[cnt] = table[i];
        his->Offset[cnt++] = tmp;
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
    vector<float> table(childNumber, 0);
    int cnt = 0;
    while (cnt <= 1)
    {
        if (cnt == 1)
            divisor /= 10;
        cnt = 0;
        table = vector<float>(childNumber, 0);
        for (int i = 0; i < dataset.size(); i++)
        {
            if (dataset[i].first != -1)
            {
                int idx = float(dataset[i].first - minValue) / divisor;
                idx = min(idx, int(table.size()) - 1);
                table[idx]++;
            }
        }
        if (table[0] > 0)
            cnt++;
        table[0] = table[0] / dataset.size() * childNumber;
        for (int i = 1; i < table.size(); i++)
        {
            if (table[i] > 0)
                cnt++;
            table[i] = table[i] / dataset.size() * childNumber + table[i - 1];
        }
    }
    table[0] = round(table[0]);
    for (int i = 1; i < childNumber; i++)
    {
        table[i] = round(table[i]);
        if (table[i] - table[i - 1] > 1)
            table[i] = table[i - 1] + 1;
    }
    cnt = 0;
    for (int i = 0; i < childNumber; i += 16)
    {
        unsigned short start_idx = int(table[i]);
        unsigned short tmp = 0;
        for (int j = i; j < i + 16; j++)
        {
            if (j >= childNumber)
            {
                while (j < i + 16)
                {
                    tmp = tmp << 1;
                    j++;
                }
                Base[cnt] = table[i];
                Offset[cnt] = tmp;
                return;
            }
            unsigned short diff = int(table[j]) - start_idx;
            tmp = (tmp << 1) + diff;
#ifdef DEBUG
            if (diff > 1)
                cout << "diff wrong, diff:" << diff << endl;
#endif // DEBUG
            if (diff > 0)
                start_idx += diff;
        }
        Base[cnt] = table[i];
        Offset[cnt++] = tmp;
    }
}

inline int HisModel::Predict(double key) const
{
    // return the idx in children
    int childNumber = flagNumber & 0x00FFFFFF;
    int idx = float(key - minValue) / divisor;
    if (idx < 0)
        idx = 0;
    else if (idx >= childNumber)
        idx = childNumber - 1;

    int base;
    int tmpIdx = idx / 16;
    base = Base[tmpIdx];
    int j = idx % 16;
    int tmp = Offset[tmpIdx] >> (15 - j);
    for (; j >= 0; j--)
    {
        base += tmp & 1;
        tmp = tmp >> 1;
    }
    return base;
}

#endif