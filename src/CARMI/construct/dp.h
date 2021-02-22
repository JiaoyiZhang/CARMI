#ifndef DP_H
#define DP_H

#include "../../params.h"
#include "../func/inlineFunction.h"
#include "node_cost_struct.h"
#include "greedy.h"
#include "dp_inner.h"
#include "dp_leaf.h"
#include <float.h>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

NodeCost CARMI::dp(const int initLeft, const int initSize, const int findLeft, const int findSize, const int insertLeft, const int insertSize)
{
    NodeCost nodeCost;
    if (initSize == 0 && findSize == 0)
    {
        nodeCost.space = 0;
        nodeCost.time = 0;
        nodeCost.isInnerNode = false;
        return nodeCost;
    }

    // construct a leaf node
    if (initSize + insertSize <= kMaxKeyNum)
        return dpLeaf(initLeft, initSize, findLeft, findSize, insertLeft, insertSize);
    else
    {
        auto res0 = dpLeaf(initLeft, initSize, findLeft, findSize, insertLeft, insertSize);
        auto res1 = dpInner(initLeft, initSize, findLeft, findSize, insertLeft, insertSize);
        if (res0.space + res0.time > res1.space + res1.space)
            return res1;
        else
            return res0;
    }
}

#endif // !DP_H