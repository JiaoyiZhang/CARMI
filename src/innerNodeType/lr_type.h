#ifndef LR_TYPE_H
#define LR_TYPE_H

#include "../params.h"

#include "../trainModel/lr.h"
#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include <vector>
#include <fstream>
using namespace std;

extern BaseNode **entireChild;

class LRType : public BaseNode
{
public:
    LRType() { flag = 'A'; };
    LRType(int c)
    {
        flag = 'A';
        childNumber = c;
    }
    void Initialize(const vector<pair<double, double>> &dataset);

    int childLeft;          // 4c Byte + 4
    LinearRegression model; // 20 Byte
    int childNumber;        // 4
};

inline void LRType::Initialize(const vector<pair<double, double>> &dataset)
{
    childLeft = allocateChildMemory(childNumber);
    if (dataset.size() == 0)
        return;

    model.Train(dataset, childNumber);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int p = model.Predict(dataset[i].first);
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

#endif