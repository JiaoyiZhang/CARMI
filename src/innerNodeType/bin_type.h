#ifndef BIN_TYPE_H
#define BIN_TYPE_H

#include "../params.h"
#include "../trainModel/binary_search_model.h"
#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include "../dataManager/child_array.h"
#include <vector>
#include <fstream>
using namespace std;

extern vector<BaseNode> entireChild;

class BSType
{
public:
    BSType()
    {
        flagNumber = (3 << 24);
    }
    BSType(int c)
    {
        flagNumber = (3 << 24) + c;
        model = BinarySearchModel(c);
    }
    void Initialize(const vector<pair<double, double>> &dataset);

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;           // 4c Byte + 4
    BinarySearchModel model; // 8c + 4
};

inline void BSType::Initialize(const vector<pair<double, double>> &dataset)
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