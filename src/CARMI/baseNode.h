#ifndef BASE_NODE_H
#define BASE_NODE_H
#include <iostream>
#include "nodes/innerNode/bs.h"
#include "nodes/innerNode/his.h"
#include "nodes/innerNode/lr.h"
#include "nodes/innerNode/plr.h"

#include "nodes/leafNode/array.h"
#include "nodes/leafNode/ga.h"
#include "nodes/leafNode/ycsb_leaf.h"
using namespace std;

union BaseNode
{
    LRModel lr;
    PLRModel nn;
    HisModel his;
    BSModel bs;

    ArrayType array;
    GappedArrayType ga;
    YCSBLeaf ycsbLeaf;

    BaseNode(){};
    ~BaseNode(){};
};

#endif // !BASE_NODE_H