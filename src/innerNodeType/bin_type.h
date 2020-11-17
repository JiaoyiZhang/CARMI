#ifndef BIN_TYPE_H
#define BIN_TYPE_H


#include "../params.h"
#include "../trainModel/binary_search_model.h"
#include "../leafNodeType/array_type.h"
#include "../leafNodeType/ga_type.h"
#include <vector>
#include <fstream>
using namespace std;

extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

class BSType
{
public:
    BSType(){};
    BSType(int c)
    {
        childNumber = c;
        model = BinarySearchModel(c);
    }
    void Initialize(const vector<pair<double, double> > &dataset);

    vector<int> child;  // 4c
    BinarySearchModel model;  // 8c + 4
    int childNumber;  // 4
};


inline void BSType::Initialize(const vector<pair<double, double> > &dataset)
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
    // ofstream outFile;
    // outFile.open("bin.csv", ios::app);
    // outFile<<"---------------------------------------------------"<<endl;
    // for(int i=0;i<childNumber;i++)
    // {
    //     outFile<<i<<":"<<perSubDataset[i].size()<<"\t";
    //     if((i+1)%10 == 0)
    //         outFile<<endl;
    // }
    // outFile<<endl;
    // outFile<<endl;
    // outFile<<endl;    

    // cout << "train second stage" << endl;
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
    vector<vector<pair<double, double>>>().swap(perSubDataset);
    
    // cout << "End train" << endl;
}

#endif