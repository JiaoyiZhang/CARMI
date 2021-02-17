#ifndef LR_TYPE_H
#define LR_TYPE_H

#include "../../../params.h"

#include "trainModel/linear_regression.h"
#include "../leafNode/array_type.h"
#include "../leafNode/ga.h"
#include "../../dataManager/child_array.h"
#include "../innerNode/lr.h"
#include <vector>
#include <fstream>
using namespace std;

extern vector<BaseNode> entireChild;

class LRType
{
public:
    LRType()
    {
        flagNumber = (0 << 24);
    };
    LRType(int c)
    {
        flagNumber = (0 << 24) + c;
    }
    void Initialize(const vector<pair<double, double>> &dataset);

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;          // 4 Byte
    LinearRegression model; // 20 Byte
};

inline void LRType::Initialize(const vector<pair<double, double>> &dataset)
{
    int childNumber = flagNumber & 0x00FFFFFF;
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

    for (int i = 0; i < childNumber; i++)
    {
        LRModel innerLR;
        innerLR.SetChildNumber(32);
        innerLR.Initialize(perSubDataset[i]);
        entireChild[childLeft + i].lr = innerLR;
    }
}

#endif