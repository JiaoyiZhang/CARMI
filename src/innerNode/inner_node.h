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
    BasicInnerNode()
    {
        isLeafNode = false;
    }
    BasicInnerNode(int childNum)
    {
        isLeafNode = false;
        childNumber = childNum;
    }
    bool IsLeaf() { return isLeafNode; }

    virtual void Initialize(const vector<pair<double, double>> &dataset){};

    virtual pair<double, double> Find(double key){};
    virtual bool Insert(pair<double, double> data){};
    virtual bool Delete(double key){};
    virtual bool Update(pair<double, double> data){};

protected:
    vector<BasicLeafNode *> children; // store the lower nodes
    int childNumber;                  // the size of the lower nodes
    bool isLeafNode;
};

#endif