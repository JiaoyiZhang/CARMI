#ifndef HIS_TYPE_H
#define HIS_TYPE_H

#include "../params.h"
#include "../trainModel/histogram.h"
#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../child_array.h"
#include <vector>
#include <fstream>
using namespace std;

extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

class HisType
{
public:
    HisType(){};
    HisType(int c)
    {
        childNumber = c;
        model = HistogramModel(c);
    }
    void Initialize(const vector<pair<double, double>> &dataset);

    int childLeft;        // 4c Byte + 4
    HistogramModel model; // 16+16c
    int childNumber;      // 4
};

inline void HisType::Initialize(const vector<pair<double, double>> &dataset)
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
}

#endif