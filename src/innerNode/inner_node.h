#ifndef INNER_NODE_H
#define INNER_NODE_H

#include <vector>
#include "../params.h"
#include "../leafNode/leaf_node.h"
#include "../../cpp-btree/btree_map.h"
using namespace std;

class BasicInnerNode
{
public:
    BasicInnerNode(){};
    BasicInnerNode(int childNum)
    {
        childNumber = childNum;
    }

    virtual void Initialize(const vector<pair<double, double>> &dataset){};

    virtual pair<double, double> Find(double key){};
    virtual bool Insert(pair<double, double> data){};
    virtual bool Delete(double key){};
    virtual bool Update(pair<double, double> data){};

protected:
    vector<void *> children;       // store the lower nodes
    vector<bool> children_is_leaf; //record whether each child node is a leaf node
    int childNumber;               // the size of the lower nodes
};

#endif