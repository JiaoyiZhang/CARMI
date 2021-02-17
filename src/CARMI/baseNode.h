#ifndef BASE_NODE_H
#define BASE_NODE_H
#include <iostream>
#include "nodes/rootNode/bin_type.h"
#include "nodes/rootNode/his_type.h"
#include "nodes/rootNode/lr_type.h"
#include "nodes/rootNode/plr_type.h"

#include "nodes/innerNode/bs.h"
#include "nodes/innerNode/his.h"
#include "nodes/innerNode/lr.h"
#include "nodes/innerNode/plr.h"

#include "nodes/leafNode/array.h"
#include "nodes/leafNode/ga.h"
#include "nodes/leafNode/ycsb_leaf.h"
using namespace std;

union CARMIRoot
{
    LRType lrRoot;
    PLRType plrRoot;
    HisType hisRoot;
    BSType bsRoot;
    CARMIRoot(){};
    ~CARMIRoot(){};
};

union BaseNode
{
    LRModel lr;
    PLRModel plr;
    HisModel his;
    BSModel bs;

    ArrayType array;
    GappedArrayType ga;
    YCSBLeaf ycsbLeaf;

    BaseNode(){};
    ~BaseNode(){};
};

#endif // !BASE_NODE_H