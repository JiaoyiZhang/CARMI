#ifndef CALCULATE_SPACE_H
#define CALCULATE_SPACE_H

#include <vector>
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
using namespace std;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

long double calculateSpace()
{
    cout << "Space of different classes (sizeof):" << endl;
    cout << "LRType:" << sizeof(LRType) << "\tlr model:" << sizeof(LinearRegression) << endl;
    cout << "NNType:" << sizeof(NNType) << "\tnn model:" << sizeof(PiecewiseLR) << endl;
    cout << "HisType:" << sizeof(HisType) << "\this model:" << sizeof(HistogramModel) << endl;
    cout << "BSType:" << sizeof(BSType) << "\tbs model:" << sizeof(BinarySearchModel) << endl;
    cout << "ArrayType:" << sizeof(ArrayType) << endl;
    cout << "GappedArrayType:" << sizeof(GappedArrayType) << endl;
    
    long double space = 0;

    for (int i = 0; i < LRVector.size(); i++)
        space += sizeof(LRType) + LRVector[i].childNumber * 4;
    for (int i = 0; i < NNVector.size(); i++)
        space += sizeof(NNType) + NNVector[i].childNumber * 4 + 192;
    for (int i = 0; i < HisVector.size(); i++)
        space += sizeof(HisType) + HisVector[i].childNumber * 5;
    for (int i = 0; i < BSVector.size(); i++)
        space += sizeof(BSType) + BSVector[i].childNumber * 12;
    space += sizeof(ArrayType) * ArrayVector.size();
    space += sizeof(GappedArrayType) * GAVector.size();
    space = space / 1024 / 1024;
    cout << "\nTOTAL SPACE: " << space << "MB" << endl;
    space += float(dataset.size()) * 16 / 1024 / 1024;
    cout << "\nTOTAL SPACE (include data): " << space << "MB" << endl;
    return space;
}

#endif // !CALCULATE_SPACE_H
