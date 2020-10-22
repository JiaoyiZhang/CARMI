#ifndef HIS_TYPE_H
#define HIS_TYPE_H

#include "../params.h"
#include "trainModel/binary_search_model.h"
#include "trainModel/histogram.h"
#include "trainModel/lr.h"
#include "trainModel/nn.h"
#include "leafNodeType/array_type.h"
#include "leafNodeType/ga_type.h"
#include <vector>
using namespace std;

extern vector<void *> index;  // store the entire index

class HisType
{
public:
    HisType(){};
    HisType(int c)
    {
        childNumber = c;
        model = HistogramModel(c);
    }
    void Initialize(const vector<pair<double, double>> &dataset, int childNum);

    vector<int> child;
    HistogramModel model;
    int childNumber;
};


void HisType::Initialize(const vector<pair<double, double>> &dataset, int childNum)
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
                index.push_back((void *)new ArrayType(kThreshold));
                int idx = index.size()-1;
                child.push_back((kLeafNodeID << 28) + idx);
                ((ArrayType*)(index[idx]))->SetDataset(perSubDataset[i]);
            }
            break;
        case 1:
            for(int i=0;i<childNumber;i++)
            {
                index.push_back((void *)new GappedArrayType(kThreshold));
                int idx = index.size()-1;
                child.push_back((kLeafNodeID << 28) + idx);
                ((GappedArrayType*)(index[idx]))->SetDataset(perSubDataset[i]);
            }
            break;
    }
    
    cout << "End train" << endl;
}

#endif