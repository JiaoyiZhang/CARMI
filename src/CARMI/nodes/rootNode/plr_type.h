#ifndef NN_TYPE_H
#define NN_TYPE_H

#include "../../../params.h"
#include "trainModel/piecewiseLR.h"
#include "../leafNode/array_type.h"
#include "../leafNode/ga.h"
#include "../../dataManager/child_array.h"
#include <vector>
#include <fstream>
using namespace std;

extern vector<BaseNode> entireChild;

class PLRType
{
public:
    PLRType() = default;
    // {
    //     flagNumber = (1 << 24);
    // }
    PLRType(int c)
    {
        flagNumber = (1 << 24) + c;
    }
    void Initialize(const vector<pair<double, double>> &dataset);

    int flagNumber; // 4 Byte (flag + childNumber)

    int childLeft;     // 4 Byte
    PiecewiseLR model; // 24*8+4 Byte
};

inline void PLRType::Initialize(const vector<pair<double, double>> &dataset)
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
        PLRModel innerNN;
        innerNN.SetChildNumber(32);
        innerNN.Initialize(perSubDataset[i]);
        entireChild[childLeft + i].nn = innerNN;
    }
}

#endif