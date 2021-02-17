#ifndef CALCULATE_SPACE_H
#define CALCULATE_SPACE_H

#include <vector>
#include "../nodes/rootNode/bin_type.h"
#include "../nodes/rootNode/his_type.h"
#include "../nodes/rootNode/lr_type.h"
#include "../nodes/rootNode/plr_type.h"

#include "../nodes/innerNode/bs_model.h"
#include "../nodes/innerNode/lr_model.h"
#include "../nodes/innerNode/plr_model.h"
#include "../nodes/innerNode/his_model.h"

#include "../nodes/leafNode/ga_type.h"
#include "../nodes/leafNode/array_type.h"
#include "../nodes/leafNode/ycsb_leaf.h"
using namespace std;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;

long double calculateSpace()
{
    cout << "Space of different classes (sizeof):" << endl;
    cout << "LRType:" << sizeof(LRType) << "\tlr model:" << sizeof(LinearRegression) << endl;
    cout << "PLRType:" << sizeof(PLRType) << "\tnn model:" << sizeof(PiecewiseLR) << endl;
    cout << "HisType:" << sizeof(HisType) << "\this model:" << sizeof(HistogramModel) << endl;
    cout << "BSType:" << sizeof(BSType) << "\tbs model:" << sizeof(BinarySearchModel) << endl;
    cout << "LRModel:" << sizeof(LRModel) << endl;
    cout << "PLRModel:" << sizeof(PLRModel) << endl;
    cout << "HisModel:" << sizeof(HisModel) << endl;
    cout << "BSModel:" << sizeof(BSModel) << endl;
    cout << "ArrayType:" << sizeof(ArrayType) << endl;
    cout << "GappedArrayType:" << sizeof(GappedArrayType) << endl;
    cout << "YCSBLeaf:" << sizeof(YCSBLeaf) << endl;

    long double space = 0;

    switch (kInnerNodeID)
    {
    case 0:
        space += sizeof(LRType);
        break;
    case 1:
        space += sizeof(PLRType);
        break;
    case 2:
        space += sizeof(HisType);
        break;
    case 3:
        space += sizeof(BSType);
        break;
    }

    space += 64 * nowChildNumber;
    if (!kIsYCSB)
    {
        for (int i = 0; i < nowChildNumber; i++)
        {
            if ((entireChild[i].lr.flagNumber >> 24) == 8)
                space += entireChild[i].array.m_capacity * 16;
            else if ((entireChild[i].lr.flagNumber >> 24) == 9)
                space += entireChild[i].ga.capacity * 16;
        }
    }
    space = space / 1024 / 1024;
    cout << "\tStructure SPACE: " << space << "MB" << endl;
    cout << "\nTOTAL SPACE (include data): " << space << "MB" << endl;
    return space;
}

#endif // !CALCULATE_SPACE_H
