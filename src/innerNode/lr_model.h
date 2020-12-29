#ifndef LR_MODEL_H
#define LR_MODEL_H

#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include "../baseNode.h"
using namespace std;

extern BaseNode **entireChild;

class LRModel : public BaseNode
{
public:
    LRModel() { flag = 0; };
    LRModel(int c)
    {
        flag = 0;
        childNumber = c;
        bound = childNumber / 5;
    }
    void Initialize(const vector<pair<double, double>> &dataset);
    void Train(const vector<pair<double, double>> &dataset);
    int Predict(double key);

    int childLeft;               // 4 Byte
    int childNumber;             // 4 Byte
    float divisor;               // 4 Byte
    float bound;                 // 4 Byte
    float minValue;              // 4 Byte
    pair<float, float> theta[5]; // 40 Byte
};

inline void LRModel::Initialize(const vector<pair<double, double>> &dataset)
{
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
            entireChild[childLeft + i] = new ArrayType(kThreshold);
            ((ArrayType *)entireChild[childLeft + i])->SetDataset(perSubDataset[i], kMaxKeyNum);
        }
        break;
    case 1:
        for (int i = 0; i < childNumber; i++)
        {
            entireChild[childLeft + i] = new GappedArrayType(kThreshold);
            ((GappedArrayType *)entireChild[childLeft + i])->SetDataset(perSubDataset[i], kMaxKeyNum);
        }
        break;
    }

    vector<vector<pair<double, double>>>().swap(perSubDataset);
}

inline void LRModel::Train(const vector<pair<double, double>> &dataset)
{
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
    divisor = float(maxValue - minValue) / 5;

    int i = 0;
    for (int k = 1; k <= 5; k++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        for (; i < dataset.size() - 1; i++)
        {
            if ((dataset[i].first - minValue) / divisor == k)
                break;
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
        theta[k - 1] = {theta1, theta2};
    }
}

inline int LRModel::Predict(double key)
{
    int idx = float(key - minValue) / divisor;
    if (idx < 0)
        idx = 0;
    else if (idx >= 5)
        idx = 4;
    // return the predicted idx in the children
    int p = theta[idx].first * key + theta[idx].second;
    int left = bound * idx;
    if (p < left)
        p = left;
    else if (p >= left + bound)
        p = left + bound - 1;
    return p;
}

#endif