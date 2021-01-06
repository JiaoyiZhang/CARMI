#ifndef HIS_TYPE_H
#define HIS_TYPE_H

#include "../params.h"
#include "../trainModel/histogram.h"
#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include <vector>
#include <fstream>
using namespace std;

extern vector<BaseNode> entireChild;

class HisType
{
public:
    HisType()
    {
        flagNumber = (2 << 24);
    };
    HisType(int c)
    {
        flagNumber = (2 << 24) + c;
        model = HistogramModel(c);
    }
    void Initialize(const vector<pair<double, double>> &dataset);

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;        // 4 Byte
    HistogramModel model; // 16+16c
};

inline void HisType::Initialize(const vector<pair<double, double>> &dataset)
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

#endif