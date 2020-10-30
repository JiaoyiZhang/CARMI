#ifndef HIS_TYPE_H
#define HIS_TYPE_H


#include "../params.h"
#include "../trainModel/binary_search_model.h"
#include "../trainModel/histogram.h"
#include "../trainModel/lr.h"
#include "../trainModel/nn.h"
#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include <vector>
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
    void Initialize(const vector<pair<double, double> > &dataset, int childNum);

    vector<int> child;
    HistogramModel model;
    int childNumber;
};


inline void HisType::Initialize(const vector<pair<double, double> > &dataset, int childNum)
{
    if (dataset.size() == 0)
        return;

    model.Train(dataset);

    vector<vector<pair<double, double> > > perSubDataset;
    vector<pair<double, double> > tmp;
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
                ArrayVector.push_back(ArrayType(kThreshold));
                int idx = ArrayVector.size()-1;
                child.push_back(0x40000000 + idx);
                ArrayVector[idx].SetDataset(perSubDataset[i]);
            }
            break;
        case 1:
            for(int i=0;i<childNumber;i++)
            {
                GAVector.push_back(GappedArrayType(kThreshold));
                int idx = GAVector.size()-1;
                child.push_back(0x50000000 + idx);
                GAVector[idx].SetDataset(perSubDataset[i]);
            }
            break;
    }
    
    cout << "End train" << endl;
}

#endif