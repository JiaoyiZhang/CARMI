#ifndef CALCULATE_SPACE_H
#define CALCULATE_SPACE_H

#include <vector>
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"

#include "../innerNode/bs_model.h"
#include "../innerNode/lr_model.h"
#include "../innerNode/nn_model.h"
#include "../innerNode/his_model.h"
using namespace std;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

long double calculateSpace()
{
    cout << "Space of different classes (sizeof):" << endl;
    cout << "LRType:" << sizeof(LRType) << "\tlr model:" << sizeof(LinearRegression) << endl;
    cout << "NNType:" << sizeof(NNType) << "\tnn model:" << sizeof(PiecewiseLR) << endl;
    cout << "HisType:" << sizeof(HisType) << "\this model:" << sizeof(HistogramModel) << endl;
    cout << "BSType:" << sizeof(BSType) << "\tbs model:" << sizeof(BinarySearchModel) << endl;
    cout << "LRModel:" << sizeof(LRModel) << endl;
    cout << "NNModel:" << sizeof(NNModel) << endl;
    cout << "HisModel:" << sizeof(HisModel) << endl;
    cout << "BSModel:" << sizeof(BSModel) << endl;
    cout << "ArrayType:" << sizeof(ArrayType) << endl;
    cout << "GappedArrayType:" << sizeof(GappedArrayType) << endl;

    long double space = 0;

    for (int i = 0; i < nowChildNumber; i++)
    {
        if (entireChild[i]->flag < 'E')
            space += 64;
        else if (entireChild[i]->flag == 'E')
            space += sizeof(ArrayType);
        else
            space += sizeof(GappedArrayType);
    }
    space = space / 1024 / 1024;
    cout << "\nTOTAL SPACE: " << space << "MB" << endl;
    space += float(dataset.size()) * 16 / 1024 / 1024;
    cout << "\nTOTAL SPACE (include data): " << space << "MB" << endl;
    return space;
}

#endif // !CALCULATE_SPACE_H
