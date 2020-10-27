#ifndef NN_TYPE_H
#define NN_TYPE_H


#include "../params.h"

#include "../trainModel/binary_search_model.h"
#include "../trainModel/histogram.h"
#include "../trainModel/lr.h"
#include "../trainModel/nn.h"
#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include <vector>
using namespace std;

extern vector<void *> INDEX;  // store the entire INDEX

class NNType
{
public:
    NNType(){};
    NNType(int c)
    {
        childNumber = c;
    }
    void Initialize(const vector<pair<double, double>> &dataset, int childNum);

    vector<int> child;
    Net model;
    int childNumber;
};

inline void NNType::Initialize(const vector<pair<double, double>> &dataset, int childNum)
{
    if (dataset.size() == 0)
        return;

    model.Train(dataset);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = model.Predict(dataset[i].first);
        int preIdx = static_cast<int>(p * (childNum - 1));
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    cout << "train second stage" << endl;
    switch (kLeafNodeID)
    {
        case 0:
            for(int i=0;i<childNumber;i++)
            {
                INDEX.push_back((void *)new ArrayType(kThreshold));
                int idx = INDEX.size()-1;
                child.push_back(0x40000000 + idx);
                ((ArrayType*)(INDEX[idx]))->SetDataset(perSubDataset[i]);
            }
            break;
        case 1:
            for(int i=0;i<childNumber;i++)
            {
                INDEX.push_back((void *)new GappedArrayType(kThreshold));
                int idx = INDEX.size()-1;
                child.push_back(0x50000000 + idx);
                ((GappedArrayType*)(INDEX[idx]))->SetDataset(perSubDataset[i]);
            }
            break;
    }
    
    cout << "End train" << endl;
}


#endif