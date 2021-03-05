#ifndef DP_H
#define DP_H

#include "../../params.h"
#include "../func/inlineFunction.h"
#include "structures.h"
#include "greedy.h"
#include "dp_inner.h"
#include "dp_leaf.h"
#include <float.h>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

NodeCost CARMI::dp(DataRange *range)
{
    NodeCost nodeCost;
    if (range->initRange.size == 0 && range->findRange.size == 0)
    {
        nodeCost = NodeCost(0, 0, 0, false);
        return nodeCost;
    }

    if (range->initRange.size <= kMaxKeyNum)
        return dpLeaf(range);
    else if (range->initRange.size > 4096)
        return dpInner(range);
    else
    {
        auto res0 = dpLeaf(range);
        auto res1 = dpInner(range);
        if (res0.space + res0.time > res1.space + res1.time)
            return res1;
        else
            return res0;
    }
}

#endif // !DP_H