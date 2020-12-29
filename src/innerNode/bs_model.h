#ifndef BS_MODEL_H
#define BS_MODEL_H

#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include "../baseNode.h"
using namespace std;

extern BaseNode **entireChild;

class BSModel : public BaseNode
{
public:
    BSModel() { flag = 3; };
    BSModel(int c)
    {
        flag = 3;
        childNumber = min(c, 12); // childNumber = 12
        for (int i = 0; i < 12; i++)
            index[i] = 0;
    }
    void Initialize(const vector<pair<double, double>> &dataset);
    void Train(const vector<pair<double, double>> &dataset);
    int Predict(double key);

    int childLeft;   // 4 Byte
    int childNumber; // 4 Byte
    float divisor;   // 4 Byte
    float index[12]; // 48 Byte
};

inline void BSModel::Initialize(const vector<pair<double, double>> &dataset)
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

inline void BSModel::Train(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
    float value = float(dataset.size()) / 12;
    int cnt = 1;
    for (int i = value * cnt - 1; i < dataset.size(); i = value * (++cnt) - 1)
    {
        if (dataset[i].first != -1)
        {
            if (cnt >= 12)
                cout << "in bs model, cnt >= 12, cnt:" << cnt << endl;
            index[cnt - 1] = dataset[i].first;
        }
        else
        {
            for (int j = i + 1; j < dataset.size(); j++)
            {
                if (dataset[j].first != -1)
                {
                    if (cnt >= 12)
                        cout << "in bs model, cnt >= 12, cnt:" << cnt << endl;
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
    int end_idx = childNumber - 1;
    int mid;
    while (start_idx < end_idx)
    {
        mid = (start_idx + end_idx) / 2;
        if (index[mid] < key)
            start_idx = mid + 1;
        else
            end_idx = mid;
    }
    return end_idx;
}

#endif