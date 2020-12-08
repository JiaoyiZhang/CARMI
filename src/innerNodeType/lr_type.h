#ifndef LR_TYPE_H
#define LR_TYPE_H

#include "../params.h"

#include "../trainModel/lr.h"
#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../child_array.h"
#include <vector>
#include <fstream>
using namespace std;

extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

class LRType
{
public:
    LRType(){};
    LRType(int c)
    {
        childNumber = c;
        childLeft = allocateChildMemory(c);
    }
    void Initialize(const vector<pair<double, double>> &dataset);

    int childLeft;          // 4c Byte + 4
    LinearRegression model; // 20 Byte
    int childNumber;        // 4
};

inline void LRType::Initialize(const vector<pair<double, double>> &dataset)
{
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

    // cout << "train second stage" << endl;
    switch (kLeafNodeID)
    {
    case 0:
        for (int i = 0; i < childNumber; i++)
        {
            ArrayVector.push_back(ArrayType(kThreshold));
            int idx = ArrayVector.size() - 1;
            entireChild[childLeft + i] = 0x40000000 + idx;
            ArrayVector[idx].SetDataset(perSubDataset[i], kMaxKeyNum);
        }
        break;
    case 1:
        for (int i = 0; i < childNumber; i++)
        {
            GAVector.push_back(GappedArrayType(kThreshold));
            int idx = GAVector.size() - 1;
            entireChild[childLeft + i] = 0x50000000 + idx;
            GAVector[idx].SetDataset(perSubDataset[i], kMaxKeyNum);
        }
        break;
    }

    vector<vector<pair<double, double>>>().swap(perSubDataset);

    // cout << "End train" << endl;
}

#endif