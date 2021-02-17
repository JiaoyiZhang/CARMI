#ifndef BS_MODEL_H
#define BS_MODEL_H

#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include "../baseNode.h"
#include <vector>
using namespace std;

extern vector<BaseNode> entireChild;
extern vector<pair<double, double>> findActualDataset;

inline void BSModel::Initialize(const vector<pair<double, double>> &dataset)
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

inline void BSModel::Train(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
    int childNumber = flagNumber & 0x00FFFFFF;
    float value = float(dataset.size()) / childNumber;
    int cnt = 1;
    for (int i = value * cnt - 1; i < dataset.size(); i = value * (++cnt) - 1)
    {
        if (cnt >= childNumber)
            break;
        if (dataset[i].first != -1)
        {
            index[cnt - 1] = dataset[i].first;
        }
        else
        {
            for (int j = i + 1; j < dataset.size(); j++)
            {
                if (dataset[j].first != -1)
                {
                    index[cnt - 1] = dataset[i].first;
                    break;
                }
            }
        }
    }
}

inline int BSModel::Predict(double key)
{
    int start_idx = 0;
    int end_idx = (flagNumber & 0x00FFFFFF) - 1;
    int mid;
    while (start_idx < end_idx)
    {
        mid = (start_idx + end_idx) / 2;
        if (index[mid] < key)
            start_idx = mid + 1;
        else
            end_idx = mid;
    }
    return start_idx;
}

inline void BSModel::Train(const int left, const int size)
{
    if (size == 0)
        return;
    int childNumber = flagNumber & 0x00FFFFFF;
    float value = float(size) / childNumber;
    int cnt = 1;
    int start = min(float(left), left + value * cnt - 1);
    int end = left + size;
    for (int i = start; i < end; i += value)
    {
        if (cnt >= childNumber)
            break;
        if (findActualDataset[i].first != -1)
        {
            index[cnt - 1] = findActualDataset[i].first;
        }
        else
        {
            for (int j = i + 1; j < end; j++)
            {
                if (findActualDataset[j].first != -1)
                {
                    index[cnt - 1] = findActualDataset[i].first;
                    break;
                }
            }
        }
        cnt++;
    }
}
#endif