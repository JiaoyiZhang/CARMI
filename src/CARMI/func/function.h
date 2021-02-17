#ifndef FUNCTION_H
#define FUNCTION_H

#include "../../params.h"

#include "inlineFunction.h"
#include "find_function.h"
#include "insert_function.h"
#include "delete_function.h"
#include "update_function.h"
#include "rangescan_function.h"
#include "../dataManager/datapoint.h"
#include <float.h>
#include <math.h>
#include <iomanip>
using namespace std;

extern vector<BaseNode> entireChild;

extern pair<double, double> *entireData;
extern unsigned int entireDataSize;

extern int kLeafNodeID;
extern int kInnerNodeID;

extern int kThreshold;
extern const double kDensity;
extern const int kAdaptiveChildNum;
extern double kRate;
extern int kMaxKeyNum;

extern LRType lrRoot;
extern PLRType plrRoot;
extern HisType hisRoot;
extern BSType bsRoot;

void Initialize(const vector<pair<double, double>> &dataset, int childNum)
{
    // create the root node
    switch (kInnerNodeID)
    {
    case 0:
        lrRoot = LRType(childNum);
        lrRoot.Initialize(dataset);
        break;
    case 1:
        plrRoot = PLRType(childNum);
        plrRoot.Initialize(dataset);
        break;
    case 2:
        hisRoot = HisType(childNum);
        hisRoot.Initialize(dataset);
        break;
    case 3:
        bsRoot = BSType(childNum);
        bsRoot.Initialize(dataset);
        break;
    }
}

pair<double, double> TestFind(int rootType, double key)
{
    return {};
}
#endif
