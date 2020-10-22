#ifndef FUNCTION_H
#define FUNCTION_H

#include "params.h"
#include "innerNodeType/bin_type.h"
#include "innerNodeType/his_type.h"
#include "innerNodeType/lr_type.h"
#include "innerNodeType/nn_type.h"
#include "leafNodeType/ga_type.h"
#include "leafNodeType/array_type.h"
#include <vector>
using namespace std;

extern vector<void *> index;  // store the entire index

extern int kLeafNodeID;
extern int kInnerNodeID;

extern int kThreshold;
extern const double kDensity;
extern const int kAdaptiveChildNum;
extern double kRate;
extern const double kReadWriteRate;
extern int kMaxKeyNum;


void Initialize(const vector<pair<double, double>> &dataset, int childNum)
{
    // create the root node
    switch (kInnerNodeID)
    {
    case 0:
        index.push_back((void *)new LRType(childNum));
        ((LRType *)index[0])->Initialize(dataset, childNum);
        break;
    case 1:
        index.push_back((void *)new NNType(childNum));
        ((NNType *)index[0])->Initialize(dataset, childNum);
        break;
    case 2:
        index.push_back((void *)new HisType(childNum));
        ((HisType *)index[0])->Initialize(dataset, childNum);
        break;
    case 3:
        index.push_back((void *)new BSType(childNum));
        ((BSType *)index[0])->Initialize(dataset, childNum);
        break;
    }
}

// pair<double, double> Find(double key)
// {
//     double p = model->Predict(key);
//     int preIdx = static_cast<int>(p * (childNumber - 1));
//     return {key, key};
//     // // if (children_is_leaf[preIdx] == false)
//     // //     return ((BasicInnerNode *)children[preIdx])->Find(key);
//     // // start = clock();
//     // return ((BasicLeafNode *)children[preIdx])->Find(key);
// }
// bool Insert(pair<double, double> data)
// {
//     double p = model->Predict(data.first);
//     int preIdx = static_cast<int>(p * (childNumber - 1));
//     return ((BasicLeafNode *)children[preIdx])->Insert(data);
// }
// bool Delete(double key)
// {
//     double p = model->Predict(key);
//     int preIdx = static_cast<int>(p * (childNumber - 1));
//     if (children_is_leaf[preIdx] == false)
//         return ((BasicInnerNode *)children[preIdx])->Delete(key);
//     return ((BasicLeafNode *)children[preIdx])->Delete(key);
// }
// bool Update(pair<double, double> data)
// {
//     double p = model->Predict(data.first);
//     int preIdx = static_cast<int>(p * (childNumber - 1));
//     if (children_is_leaf[preIdx] == false)
//         return ((BasicInnerNode *)children[preIdx])->Update(data);
//     return ((BasicLeafNode *)children[preIdx])->Update(data);
// }




#endif



