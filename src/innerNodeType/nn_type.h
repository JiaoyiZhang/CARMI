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
#include <fstream>
using namespace std;

extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

class NNType
{
public:
    NNType(){};
    NNType(int c)
    {
        childNumber = c;
    }
    void Initialize(const vector<pair<double, double> > &dataset);

    vector<int> child;
    Net model;
    int childNumber;
};

inline void NNType::Initialize(const vector<pair<double, double> > &dataset)
{
    if (dataset.size() == 0)
        return;

    model.Train(dataset, childNumber);

    vector<vector<pair<double, double> > > perSubDataset;
    vector<pair<double, double> > tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int p = model.Predict(dataset[i].first);
        perSubDataset[p].push_back(dataset[i]);
    }
    ofstream outFile;
    outFile.open("nn.csv", ios::app);
    outFile<<"---------------------------------------------------"<<endl;
    for(int i=0;i<childNumber;i++)
    {
        outFile<<i<<":"<<perSubDataset[i].size()<<"\t";
        if((i+1)%10 == 0)
            outFile<<endl;
    }
    outFile<<endl;
    outFile<<endl;
    outFile<<endl;    
    
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